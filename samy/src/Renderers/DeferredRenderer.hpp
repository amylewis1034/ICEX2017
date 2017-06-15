#ifndef DEFERREDRENDERER_HPP
#define DEFERREDRENDERER_HPP

#include <glm/glm.hpp>
#include "Renderer.hpp"
#include <World.hpp>
#include <Graphics/GLFramebuffer.hpp>
#include <Graphics/GLTexture.hpp>
#include <Graphics/GLShaderProgram.hpp>

class DeferredRenderer : public Renderer {
public:
    DeferredRenderer();
    ~DeferredRenderer();

    virtual void render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world);

    GLuint getPosition() const;
    GLuint getNormal() const;
    GLuint getAlbedoSpecular() const;

private:
    GLFramebuffer gBuffer;
    GLTexture defaultTexture;
    GLShaderProgram deferredShader, quadShader;
};

#endif