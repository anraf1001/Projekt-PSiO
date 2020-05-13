#include "map.hpp"

Map::Map() {
    generate();
}

void Map::generate() {
    std::vector<std::vector<char>> cells = mazeToChar(generateTilesPlacement());
    std::thread convToPolyThread(&Map::convertTileMapToPolyMap, this, cells);

    for (std::size_t i = 0; i < cells.size(); i++) {
        for (std::size_t j = 0; j < cells[i].size(); j++) {
            sf::RectangleShape box;
            box.setSize(sf::Vector2f(TILE, TILE));
            box.setPosition(static_cast<float>(j) * TILE, static_cast<float>(i) * TILE);
            box.setOrigin(TILE / 2.0f, TILE / 2.0f);
            switch (cells[i][j]) {
                case '#':
                    box.setFillColor(sf::Color::Blue);
                    m_mapGrid.emplace_back(box);
                    break;
                case 's':
                    box.setFillColor(sf::Color::Green);
                    m_mapGrid.emplace_back(box);
                    break;
                case 'e':
                    box.setFillColor(sf::Color::Red);
                    m_mapGrid.emplace_back(box);
                    break;
                default:
                    box.setFillColor(sf::Color::Yellow);
                    box.setSize(sf::Vector2f(TILE / 4.0f, TILE / 4.0f));
                    box.setOrigin(TILE / 8.0f, TILE / 8.0f);
                    m_mapGrid.emplace_back(box);
                    break;
            }
        }
    }
    convToPolyThread.join();
}

std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> Map::generateTilesPlacement() {
    std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> maze{};
    std::stack<std::pair<unsigned int, unsigned int>> backtrack;
    unsigned int visitedCells;
    std::mt19937 generator(
            static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

    std::size_t longestPath = 0;
    std::pair<unsigned int, unsigned int> endCell;

    backtrack.push(std::make_pair(0, 0));
    visitedCells = 1;
    maze[0][0].visit();
    maze[0][0].grid[1][1] = 's';

    do {
        std::vector<Direction> neighbours;

        if (backtrack.top().second > 0 && !maze[backtrack.top().second - 1][backtrack.top().first].wasVisited()) {
            neighbours.emplace_back(Direction::NORTH);
        }
        if (backtrack.top().second < MAP_HEIGHT - 1 &&
            !maze[backtrack.top().second + 1][backtrack.top().first].wasVisited()) {
            neighbours.emplace_back(Direction::SOUTH);
        }
        if (backtrack.top().first < MAP_WIDTH - 1 &&
            !maze[backtrack.top().second][backtrack.top().first + 1].wasVisited()) {
            neighbours.emplace_back(Direction::EAST);
        }
        if (backtrack.top().first > 0 && !maze[backtrack.top().second][backtrack.top().first - 1].wasVisited()) {
            neighbours.emplace_back(Direction::WEST);
        }

        if (!neighbours.empty()) {
            Direction nextDir = neighbours[generator() % neighbours.size()];

            switch (nextDir) {
                case Direction::NORTH:
                    maze[backtrack.top().second][backtrack.top().first].grid[0][1] = ' ';
                    backtrack.push(std::make_pair(backtrack.top().first, backtrack.top().second - 1));
                    maze[backtrack.top().second][backtrack.top().first].visit();
                    visitedCells++;
                    break;
                case Direction::SOUTH:
                    maze[backtrack.top().second + 1][backtrack.top().first].grid[0][1] = ' ';
                    backtrack.push(std::make_pair(backtrack.top().first, backtrack.top().second + 1));
                    maze[backtrack.top().second][backtrack.top().first].visit();
                    visitedCells++;
                    break;
                case Direction::EAST:
                    maze[backtrack.top().second][backtrack.top().first + 1].grid[1][0] = ' ';
                    backtrack.push(std::make_pair(backtrack.top().first + 1, backtrack.top().second));
                    maze[backtrack.top().second][backtrack.top().first].visit();
                    visitedCells++;
                    break;
                case Direction::WEST:
                    maze[backtrack.top().second][backtrack.top().first].grid[1][0] = ' ';
                    backtrack.push(std::make_pair(backtrack.top().first - 1, backtrack.top().second));
                    maze[backtrack.top().second][backtrack.top().first].visit();
                    visitedCells++;
                    break;
                default:
                    break;
            }
        } else {
            if (backtrack.size() >= longestPath) {
                longestPath = backtrack.size();
                endCell = backtrack.top();
            }
            backtrack.pop();
        }
    } while (visitedCells < MAP_WIDTH * MAP_HEIGHT);

    maze[endCell.second][endCell.first].grid[1][1] = 'e';

    return maze;
}

std::vector<std::vector<char>> Map::mazeToChar(const std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> &maze) {
    std::vector<std::vector<char>> cells;

    for (auto &row : maze) {
        std::vector<char> temp;
        for (auto &cell : row) {
            temp.emplace_back(cell.grid[0][0]);
            temp.emplace_back(cell.grid[0][1]);
        }
        temp.emplace_back('#');

        std::vector<char> temp2;
        for (auto &cell : row) {
            temp2.emplace_back(cell.grid[1][0]);
            temp2.emplace_back(cell.grid[1][1]);
        }
        temp2.emplace_back('#');

        cells.emplace_back(temp);
        cells.emplace_back(temp2);
    }
    std::vector<char> endingRow(MAP_WIDTH * 2 + 1, '#');
    cells.emplace_back(endingRow);

    return cells;
}

void Map::draw(sf::RenderWindow &window) const {
    for (const auto &cell : m_mapGrid) {
        window.draw(cell);
    }
}

void Map::collisionDetection(Pacman &player, bool &endTileHit) {
    sf::Vector2f direction;
    Collider playerCollider = player.getCollider();

    for (auto &box : m_mapGrid) {
        if (box.getFillColor() == sf::Color::Blue && Collider(box).checkCollision(playerCollider, direction)) {
            player.onCollision(direction);
        } else if (box.getFillColor() == sf::Color::Red && box.getGlobalBounds().intersects(player.getGlobalBounds())) {
            endTileHit = true;
        } else if (box.getFillColor() == sf::Color::Yellow &&
                   Collider(box).checkCollision(playerCollider, direction)) {
            box.setFillColor(sf::Color::Black);
        }
    }
}

void Map::addPolyMapBoundary() {
    Edge Top, Right, Bottom, Left;

    Top.start.x = TILE / 2.0f;
    Top.start.y = TILE / 2.0f;
    Top.end.x = Top.start.x + 2 * TILE * (MAP_WIDTH - 0.5f);
    Top.end.y = Top.start.y;

    Right.start.x = Top.end.x;
    Right.start.y = Top.end.y;
    Right.end.x = Right.start.x;
    Right.end.y = Right.start.y + 2 * TILE * (MAP_HEIGHT - 0.5f);

    Bottom.start.x = Right.end.x;
    Bottom.start.y = Right.end.y;
    Bottom.end.x = Bottom.start.x - 2 * TILE * (MAP_WIDTH - 0.5f);
    Bottom.end.y = Bottom.start.y;

    Left.start.x = Bottom.end.x;
    Left.start.y = Bottom.end.y;
    Left.end.x = Left.start.x;
    Left.end.y = Left.start.y - 2 * TILE * (MAP_HEIGHT - 0.5f);

    m_edges.emplace_back(Top);
    m_edges.emplace_back(Right);
    m_edges.emplace_back(Bottom);
    m_edges.emplace_back(Left);
}

void Map::convertTileMapToPolyMap(const std::vector<std::vector<char>> &cellInChars) {
    std::mutex m;
    const std::lock_guard<std::mutex> lock(m);
    std::vector<std::vector<polyCell>> cells;

    for (const auto &row : cellInChars) {
        std::vector<polyCell> temp;
        for (const auto &x :row) {
            polyCell cell;
            if (x == '#') cell.exist = true;
            temp.emplace_back(cell);
        }
        cells.emplace_back(temp);
    }

    addPolyMapBoundary();

    for (unsigned int i = 1; i < cells.size() - 1; i++) {
        for (unsigned int j = 1; j < cells[i].size() - 1; j++) {

            if (cells[i][j].exist) {
                // If cell has no western neighbour
                if (!cells[i][j - 1].exist) {
                    // It can extend an edge from its northern neighbour if it has one,
                    // or it can start a new one
                    if (cells[i - 1][j].edge[Direction::WEST].second) {
                        m_edges[cells[i - 1][j].edge[Direction::WEST].first].end.y += TILE;
                        cells[i][j].edge[Direction::WEST].first = cells[i - 1][j].edge[Direction::WEST].first;
                        cells[i][j].edge[Direction::WEST].second = true;
                    } else {
                        Edge edge;
                        edge.start.x = static_cast<float>(j) * TILE - TILE / 2;
                        edge.start.y = static_cast<float>(i) * TILE - TILE / 2;
                        edge.end.x = edge.start.x;
                        edge.end.y = edge.start.y + TILE;

                        std::size_t edgeID = m_edges.size();
                        m_edges.emplace_back(edge);

                        cells[i][j].edge[Direction::WEST].first = edgeID;
                        cells[i][j].edge[Direction::WEST].second = true;
                    }
                }

                // If cell has no eastern neighbour
                if (!cells[i][j + 1].exist) {
                    if (cells[i - 1][j].edge[Direction::EAST].second) {
                        m_edges[cells[i - 1][j].edge[Direction::EAST].first].end.y += TILE;
                        cells[i][j].edge[Direction::EAST].first = cells[i - 1][j].edge[Direction::EAST].first;
                        cells[i][j].edge[Direction::EAST].second = true;
                    } else {
                        Edge edge;
                        edge.start.x = static_cast<float>(j + 1) * TILE - TILE / 2;
                        edge.start.y = static_cast<float>(i) * TILE - TILE / 2;
                        edge.end.x = edge.start.x;
                        edge.end.y = edge.start.y + TILE;

                        std::size_t edgeID = m_edges.size();
                        m_edges.emplace_back(edge);

                        cells[i][j].edge[Direction::EAST].first = edgeID;
                        cells[i][j].edge[Direction::EAST].second = true;
                    }
                }

                // If cell has no northern neighbour
                if (!cells[i - 1][j].exist) {
                    if (cells[i][j - 1].edge[Direction::NORTH].second) {
                        m_edges[cells[i][j - 1].edge[Direction::NORTH].first].end.x += TILE;
                        cells[i][j].edge[Direction::NORTH].first = cells[i][j - 1].edge[Direction::NORTH].first;
                        cells[i][j].edge[Direction::NORTH].second = true;
                    } else {
                        Edge edge;
                        edge.start.x = static_cast<float>(j) * TILE - TILE / 2;
                        edge.start.y = static_cast<float>(i) * TILE - TILE / 2;
                        edge.end.x = edge.start.x + TILE;
                        edge.end.y = edge.start.y;

                        std::size_t edgeID = m_edges.size();
                        m_edges.emplace_back(edge);

                        cells[i][j].edge[Direction::NORTH].first = edgeID;
                        cells[i][j].edge[Direction::NORTH].second = true;
                    }
                }

                // If cell has no southern neighbour
                if (!cells[i + 1][j].exist) {
                    if (cells[i][j - 1].edge[Direction::SOUTH].second) {
                        m_edges[cells[i][j - 1].edge[Direction::SOUTH].first].end.x += TILE;
                        cells[i][j].edge[Direction::SOUTH].first = cells[i][j - 1].edge[Direction::SOUTH].first;
                        cells[i][j].edge[Direction::SOUTH].second = true;
                    } else {
                        Edge edge;
                        edge.start.x = static_cast<float>(j) * TILE - TILE / 2;
                        edge.start.y = static_cast<float>(i + 1) * TILE - TILE / 2;
                        edge.end.x = edge.start.x + TILE;
                        edge.end.y = edge.start.y;

                        std::size_t edgeID = m_edges.size();
                        m_edges.emplace_back(edge);

                        cells[i][j].edge[Direction::SOUTH].first = edgeID;
                        cells[i][j].edge[Direction::SOUTH].second = true;
                    }
                }
            }
        }
    }
}

void Map::checkVisibility(const sf::Vector2f &playerPos) {
    m_visiblePolyPoints.clear();

    for (const auto &edge : m_edges) {
        for (int i = 0; i < 2; i++) {
            sf::Vector2f ray((i == 0 ? edge.start.x : edge.end.x) - playerPos.x,
                             (i == 0 ? edge.start.y : edge.end.y) - playerPos.y);

            float base_angle = atan2f(ray.y, ray.x);

            float angle;
            for (int j = 0; j < 3; j++) {
                if (j == 0) angle = base_angle - 0.0001f;
                if (j == 1) angle = base_angle;
                if (j == 2) angle = base_angle + 0.0001f;

                ray.x = cosf(angle);
                ray.y = sinf(angle);

                float min_t1 = INFINITY;
                sf::Vector2f rayEndPoint(0.0f, 0.0f);
                float rayAngle = 0.0f;
                bool hitSomething = false;

                for (const auto &edge2 : m_edges) {
                    sf::Vector2f segment(edge2.end.x - edge2.start.x,
                                         edge2.end.y - edge2.start.y);

                    if (std::abs(segment.x - ray.x) > 0.0f && std::abs(segment.y - ray.y) > 0.0f) {
                        float t2 = (ray.x * (edge2.start.y - playerPos.y) + (ray.y * (playerPos.x - edge2.start.x))) /
                                   (segment.x * ray.y - segment.y * ray.x);
                        float t1 = (edge2.start.x + segment.x * t2 - playerPos.x) / ray.x;

                        if (t1 > 0.0f && t2 >= 0.0f && t2 <= 1.0f) {
                            if (t1 < min_t1) {
                                min_t1 = t1;
                                rayEndPoint.x = playerPos.x + ray.x * t1;
                                rayEndPoint.y = playerPos.y + ray.y * t1;
                                rayAngle = atan2f(rayEndPoint.y - playerPos.y, rayEndPoint.x - playerPos.x);
                                hitSomething = true;
                            }
                        }
                    }
                }
                if (hitSomething) {
                    m_visiblePolyPoints.emplace_back(std::make_pair(rayAngle, rayEndPoint));
                }
            }
        }
    }
    std::sort(m_visiblePolyPoints.begin(), m_visiblePolyPoints.end(),
              [](const std::pair<float, sf::Vector2f> &p1, const std::pair<float, sf::Vector2f> &p2) {
                  return p1.first < p2.first;
              });
}

void Map::drawLight(const sf::Vector2f &playerPos, sf::RenderWindow &window) {
    auto it = std::unique(m_visiblePolyPoints.begin(), m_visiblePolyPoints.end(),
                          [](const std::pair<float, sf::Vector2f> &p1, const std::pair<float, sf::Vector2f> &p2) {
                              return std::abs(p1.second.x - p2.second.x) < 0.1f &&
                                     std::abs(p1.second.y - p2.second.y) < 0.1f;
                          });
    m_visiblePolyPoints.erase(it, m_visiblePolyPoints.end());

    std::vector<sf::VertexArray> lightTriangles;

    sf::Color lightColor(255, 202, 3, 80);

    if (m_visiblePolyPoints.size() > 1) {
        for (std::size_t i = 0; i < m_visiblePolyPoints.size() - 1; i++) {
            sf::VertexArray triangle(sf::Triangles, 3);

            triangle[0].position = playerPos;
            triangle[1].position = m_visiblePolyPoints[i].second;
            triangle[2].position = m_visiblePolyPoints[i + 1].second;

            triangle[0].color = lightColor;
            triangle[1].color = lightColor;
            triangle[2].color = lightColor;

            lightTriangles.emplace_back(triangle);
        }
        sf::VertexArray lastTriangle(sf::Triangles, 3);
        lastTriangle[0].position = playerPos;
        lastTriangle[1].position = m_visiblePolyPoints[m_visiblePolyPoints.size() - 1].second;
        lastTriangle[2].position = m_visiblePolyPoints[0].second;

        lastTriangle[0].color = lightColor;
        lastTriangle[1].color = lightColor;
        lastTriangle[2].color = lightColor;

        lightTriangles.emplace_back(lastTriangle);
    }

    for (const auto &triangle : lightTriangles) {
        window.draw(triangle);
    }
}
