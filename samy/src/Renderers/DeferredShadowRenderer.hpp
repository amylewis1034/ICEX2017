#ifndef DEFERREDSHADOWRENDERER_HPP
#define DEFERREDSHADOWRENDERER_HPP

#include <glm/glm.hpp>
#include "Renderer.hpp"
#include <World.hpp>
#include <Graphics/GLFramebuffer.hpp>
#include <Graphics/GLTexture.hpp>
#include <Graphics/GLShaderProgram.hpp>

class DeferredShadowRenderer : public Renderer {
public:
    DeferredShadowRenderer();
    virtual ~DeferredShadowRenderer();

    virtual void render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world);

    GLuint getPosition() const;
    GLuint getNormal() const;
    GLuint getAlbedoSpecular() const;

    GLuint getLightDepth() const;

private:
    GLFramebuffer gBuffer, lightFBO;
    GLTexture defaultTexture;
    GLShaderProgram deferredShader, shadowmapShader, quadShader, dirlightShader, pointlightShader, lightStencilShader;
    GLShaderProgram causticShader;
};

#endif