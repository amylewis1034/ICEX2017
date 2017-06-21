#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class World;


class Renderer {
public:
    virtual ~Renderer() {};
    virtual void render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world) = 0;
};

#endif