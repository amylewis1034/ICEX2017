#include "PostprocessRenderer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <World.hpp>
#include <Graphics/GLFramebuffer.hpp>
#include <Graphics/GLTexture.hpp>
#include <Graphics/GLShaderProgram.hpp>
#include <Graphics/GLQuad.hpp>
#include <Components/Transform.hpp>
#include <Components/Mesh.hpp>
#include <Components/Heightmap.hpp>
#include <Components/Texture.hpp>
#include <icex_common.hpp>
#include <Renderers/DeferredShadowRenderer.hpp>
#include "GLFW/glfw3.h"

#include <GL/glew.h>

extern GLFWwindow *window;

PostprocessRenderer::PostprocessRenderer() {
    /* Initialize geometry buffer */
    GLsizei width, height;
    glfwGetFramebufferSize(window, &width, &height);

    postprocessFBO.bind();
    postprocessFBO.attachTexture(
        GL_COLOR_ATTACHMENT0,
        GL_RGBA16F,
        width,
        height,
        GL_RGBA,
        GL_FLOAT
    );
    GLuint att[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, att);

    postprocessFBO.attachTexture(
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_DEPTH24_STENCIL8,
        width,
        height,
        GL_DEPTH_STENCIL,
        GL_UNSIGNED_INT_24_8,
        GL_LINEAR,
        GL_LINEAR
    );
    

    if (postprocessFBO.getStatus() != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not created successfully" << std::endl;
    }
    postprocessFBO.unbind();

	for (int i = 0; i < 2; i++) {
		bloomFBO[i].bind();
		bloomFBO[i].attachTexture(
			GL_COLOR_ATTACHMENT0,
			GL_RGBA16F,
			width,
			height,
			GL_RGBA,
			GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE
		);
		GLuint bloomAtt[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, bloomAtt);
		bloomFBO[i].unbind();
	}

    quadShader.linkProgram(SHADER_PATH "textured_quad.vert", SHADER_PATH "postprocess.frag");
	bloomShader.linkProgram(SHADER_PATH "textured_quad.vert", SHADER_PATH "bloom.frag");
	blurShader.linkProgram(SHADER_PATH "textured_quad.vert", SHADER_PATH "blur.frag");

    /* Initialize quad */
    GLQuad::init();

    this->gamma = 0.0f;
    this->exposure = 0.678f;
    this->fog_color = glm::vec3(0.7f);
    this->be = glm::vec3(0.04f);
    this->bi = glm::vec3(0.01f);
}

PostprocessRenderer::~PostprocessRenderer() {}

void PostprocessRenderer::render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world) {
    if (GLEW_KHR_debug) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Postprocessing Pass");
    }

	int width, height;
    glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

    /* Place existing scene into postprocess framebuffer (do directly in previous render pass?) */
    // glBindFramebuffer(GL_READ_FRAMEBUFFER, postprocessFBO.getHandle());
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    // glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    // return;

    glm::mat4 p, v, m;
    glClearColor(1, 0, 0, 1);
	bloomFBO[1].bind();
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
	/* Extract bright areas */
	bloomShader.bind();
	glUniformMatrix4fv(bloomShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(p));
	glUniformMatrix4fv(bloomShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(v));
	glUniformMatrix4fv(bloomShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(m));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, postprocessFBO.getTexture(0));
	glUniform1i(bloomShader.uniformLocation("color_in"), 0);

	GLQuad::draw();
	bloomShader.unbind();
	glBindTexture(GL_TEXTURE_2D, 0);

    bloomFBO[0].bind();
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    bloomFBO[0].unbind();

	/* Gaussian blur */
	blurShader.bind();
	const int blurs = 0;
	for (int i = 0; i < blurs; i++) {
		bloomFBO[i % 2].bind();
		glUniformMatrix4fv(blurShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(p));
		glUniformMatrix4fv(blurShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(blurShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(m));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bloomFBO[(i + 1) % 2].getTexture(0));
		glUniform1i(blurShader.uniformLocation("color_in"), 0);

		glUniform1i(blurShader.uniformLocation("horizontal"), i % 2);

		GLQuad::draw();
		bloomFBO[i % 2].unbind();
	}
	blurShader.unbind();

    /* Run final postprocessing (composition, gamma correction, tone mapping, motion blur) */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    quadShader.bind();
    glUniformMatrix4fv(quadShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(p));
    glUniformMatrix4fv(quadShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(v));
    glUniformMatrix4fv(quadShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(m));

    glUniform3fv(quadShader.uniformLocation("eye"), 1, glm::value_ptr(eye));

    postprocessFBO.bindTextures();
    glUniform1i(quadShader.uniformLocation("color_in"), 0);
    glUniform1i(quadShader.uniformLocation("depth"), 1);

	glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, bloomFBO[1].getTexture(0));
	glUniform1i(quadShader.uniformLocation("bloom"), 2);

    glUniform1f(quadShader.uniformLocation("gamma"), this->gamma);
    glUniform1f(quadShader.uniformLocation("exposure"), this->exposure);
    glUniform3fv(quadShader.uniformLocation("fog_color"), 1, glm::value_ptr(this->fog_color));
    glUniform3fv(quadShader.uniformLocation("be"), 1, glm::value_ptr(this->be));
    glUniform3fv(quadShader.uniformLocation("bi"), 1, glm::value_ptr(this->bi));

    static glm::mat4 lastVP = projection * view;
    float blurScale = 0.5f;
    static const DeferredShadowRenderer *dr = dynamic_cast<const DeferredShadowRenderer *>(world.getRenderer());
    if (dr != nullptr) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, dr->getPosition());
        glUniform1i(quadShader.uniformLocation("worldPos"), 3);
        glUniformMatrix4fv(quadShader.uniformLocation("lastVP"), 1, GL_FALSE, glm::value_ptr(lastVP));
        glUniform1f(quadShader.uniformLocation("blurScale"), blurScale);
        lastVP = projection * view;
    }

    glUniform1f(quadShader.uniformLocation("near"), world.getNear());
    glUniform1f(quadShader.uniformLocation("far"), world.getFar());

    GLQuad::draw();
    postprocessFBO.unbindTextures();
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    quadShader.unbind();
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, postprocessFBO.getHandle());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (GLEW_KHR_debug) {
        glPopDebugGroup();
    }
}

GLuint PostprocessRenderer::getBloomFBO() const {
    return bloomFBO[1].getHandle();
}

GLuint PostprocessRenderer::getFBO() const {
    return postprocessFBO.getHandle();
}
