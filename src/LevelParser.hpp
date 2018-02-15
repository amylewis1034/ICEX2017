#ifndef LEVELPARSER_HPP
#define LEVELPARSER_HPP

#include <World.hpp>
#include <string>

class LevelParser {
public:
    static void ParseWorld(World *world, const std::string &filename);
};

#endif