#ifndef POSTPROCESSRENDERER_HPP
#define POSTPROCESSRENDERER_HPP

#include <glm/glm.hpp>
#include "Renderer.hpp"
#include <World.hpp>
#include <Graphics/GLFramebuffer.hpp>
#include <Graphics/GLShaderProgram.hpp>

class PostprocessRenderer : public Renderer {
public:
    PostprocessRenderer();
    ~PostprocessRenderer();

    virtual void render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world);

    GLuint getBloomFBO() const;
    GLuint getFBO() const;

    float gamma, exposure;
    glm::vec3 fog_color, be, bi;
private:
    GLFramebuffer postprocessFBO, bloomFBO[2];
    GLShaderProgram quadShader, bloomShader, blurShader;
};

#endif