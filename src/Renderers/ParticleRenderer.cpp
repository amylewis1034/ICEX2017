#include "ParticleRenderer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <World.hpp>
#include <Graphics/GLFramebuffer.hpp>
#include <Graphics/GLShaderProgram.hpp>
#include <Graphics/GLQuad.hpp>
#include <Components/Transform.hpp>
#include <Components/Particle.hpp>
#include <icex_common.hpp>
#include <Renderers/DeferredShadowRenderer.hpp>
#include <Renderers/PostprocessRenderer.hpp>
#include "GLFW/glfw3.h"

#include <GL/glew.h>

extern GLFWwindow *window;

ParticleRenderer::ParticleRenderer() {
    /* Initialize geometry buffer */
    GLsizei width, height;
    glfwGetFramebufferSize(window, &width, &height);

    particleShader.linkProgram(SHADER_PATH "particle.vert", SHADER_PATH "particle.frag");

    /* Initialize quad */
    GLQuad::init();
}

ParticleRenderer::~ParticleRenderer() {}

void ParticleRenderer::render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world) {
    if (GLEW_KHR_debug) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Particle Pass");
    }

	int width, height;
    glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

    static const PostprocessRenderer *pr = dynamic_cast<const PostprocessRenderer *>(world.getPostrenderer());
    GLuint nextFBO = 0;
    if (pr != nullptr) {
        nextFBO = pr->getFBO();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);
    glEnable(GL_DEPTH_TEST);
  //  glEnable(GL_BLEND);
   // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glPointSize(6.0f);

    particleShader.bind();
    glUniformMatrix4fv(particleShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(particleShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(particleShader.uniformLocation("eye"), 1, glm::value_ptr(eye));

    for (auto p : world.getParticleSystems()) {
        Transform *t = p->getComponent<Transform>();
        Particle *ps = p->getComponent<Particle>();

        glm::mat4 model = t->getMatrix();
		glUniformMatrix4fv(particleShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));

        // Draw the points here
        glBindVertexArray(ps->VAO);
        glDrawArrays(GL_POINTS, 0, ps->numP);
    }
    glBindVertexArray(0);
    particleShader.unbind();

    glDepthMask(GL_TRUE);
  //  glDisable(GL_BLEND);

    if (GLEW_KHR_debug) {
        glPopDebugGroup();
    }
}
