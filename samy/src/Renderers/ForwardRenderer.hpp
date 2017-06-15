#ifndef FORWARDRENDERER_HPP
#define FORWARDRENDERER_HPP

#include <glm/glm.hpp>
#include "Renderer.hpp"
#include <World.hpp>
#include <Graphics/GLShaderProgram.hpp>
#include <Components/Material.hpp>
#include <Components/Shader.hpp>

class ForwardRenderer : public Renderer {
public:
    ForwardRenderer();
    ~ForwardRenderer();

    virtual void render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world);

private:
    Material defaultMaterial;
    Shader defaultShader;
    GLShaderProgram program;
};

#endif