#ifndef PROJEKT_MAP_HPP
#define PROJEKT_MAP_HPP

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <random>
#include <chrono>
#include <stack>
#include <map>
#include <thread>
#include <mutex>
#include <algorithm>
#include "cell.hpp"
#include "consts.hpp"
#include "pacman.hpp"

class Map {
public:
    Map();

    ~Map() = default;

    void draw(sf::RenderWindow &window) const;

    void collisionDetection(Pacman &player, bool &endTileHit);

    void checkVisibility(const sf::Vector2f &playerPos);

    void drawLight(const sf::Vector2f &playerPos, sf::RenderWindow &window);

private:
    enum class Direction {
        NORTH,
        SOUTH,
        EAST,
        WEST
    };

    struct Edge {
        sf::Vector2f start;
        sf::Vector2f end;
    };

    struct polyCell {
        std::map<Direction, std::pair<std::size_t, bool>> edge{{Direction::NORTH, std::make_pair(0, false)},
                                                               {Direction::SOUTH, std::make_pair(0, false)},
                                                               {Direction::EAST,  std::make_pair(0, false)},
                                                               {Direction::WEST,  std::make_pair(0, false)}};
        bool exist = false;
    };

    std::vector<sf::RectangleShape> m_mapGrid;

    std::vector<Edge> m_edges;

    std::vector<std::pair<float, sf::Vector2f>> m_visiblePolyPoints; //angle, x, y

    std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> generateTilesPlacement();

    std::vector<std::vector<char>> mazeToChar(const std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> &maze);

    void convertTileMapToPolyMap(const std::vector<std::vector<char>> &cells);

    void addPolyMapBoundary();

    void sortAndEraseDuplicatesVisiblePoints();
};

#endif //PROJEKT_MAP_HPP
