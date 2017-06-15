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

    void setGamma(float gamma);
    void setExposure(float exposure);
    void setFogDensity(float fogDensity);
    void setTen(float ten);
    void setFactor1(float factor1);
    void setFactor2(float factor2);
    float getGamma() const;
    float getExposure() const;
    float getFogDensity() const;
    float getTen() const;
    float getFactor1() const;
    float getFactor2() const;
private:
    GLFramebuffer postprocessFBO, bloomFBO[2];
    GLShaderProgram quadShader, bloomShader, blurShader;

    float gamma, exposure, fogDensity, ten, factor1, factor2;
};

#endif