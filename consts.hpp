#ifndef PROJEKT_CONSTS_HPP
#define PROJEKT_CONSTS_HPP

constexpr auto WINDOW_HEIGHT = 600;
constexpr auto WINDOW_WIDTH = 800;
constexpr auto TILE = 80.0f;
constexpr auto MAP_WIDTH = 10;
constexpr auto MAP_HEIGHT = MAP_WIDTH;

enum class Direction {
    NORTH,
    SOUTH,
    EAST,
    WEST
};

constexpr auto LIGHT_DEBUG = false;

#endif //PROJEKT_CONSTS_HPP
