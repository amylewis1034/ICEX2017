#ifndef PARTICLERENDERER_HPP
#define PARTICLERENDERER_HPP

#include <glm/glm.hpp>
#include "Renderer.hpp"
#include <World.hpp>
#include <Graphics/GLShaderProgram.hpp>

class ParticleRenderer : public Renderer {
public:
    ParticleRenderer();
    ~ParticleRenderer();

    virtual void render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world);

private:
    GLShaderProgram particleShader;
};

#endif