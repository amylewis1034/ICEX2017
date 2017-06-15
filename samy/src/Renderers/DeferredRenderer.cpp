#include "DeferredRenderer.hpp"
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
#include "GLFW/glfw3.h"

#include <GL/glew.h>

extern GLFWwindow *window;

DeferredRenderer::DeferredRenderer() {
    /* Initialize geometry buffer */
    GLsizei width, height;
    glfwGetFramebufferSize(window, &width, &height);

    gBuffer.bind();

    /* Attach position buffer */
    gBuffer.attachTexture(
        GL_COLOR_ATTACHMENT0,
        GL_RGB16F,
        width,
        height,
        GL_RGB,
        GL_FLOAT
    );

    /* Attach normal buffer */
    gBuffer.attachTexture(
        GL_COLOR_ATTACHMENT1,
        GL_RGB16F,
        width,
        height,
        GL_RGB,
        GL_FLOAT
    );
    
	/* Attach abledo + specular buffer */
    gBuffer.attachTexture(
        GL_COLOR_ATTACHMENT2,
        GL_RGBA,
        width,
        height,
        GL_RGBA,
        GL_UNSIGNED_BYTE
    );
    
    /* Tell OpenGL about number of attachments */
    GLuint att[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, att);

    gBuffer.attachRenderbuffer(
        GL_DEPTH_ATTACHMENT,
        GL_DEPTH_COMPONENT,
        width,
        height
    );

    if (gBuffer.getStatus() != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not created successfully" << std::endl;
    }
    gBuffer.unbind();

    /* Initialize default texture */
    defaultTexture.loadTexture(RESOURCE_PATH "textures/default.png");

    /* Initialize shaders */
    deferredShader.linkProgram(SHADER_PATH "phong.vert", SHADER_PATH "geometry_pass.frag");
    quadShader.linkProgram(SHADER_PATH "textured_quad.vert", SHADER_PATH "deferred_ubo.frag");

    /* Initialize quad */
    GLQuad::init();
}

DeferredRenderer::~DeferredRenderer() {}

void DeferredRenderer::render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world) {
	int width, height;
    glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glEnable(GL_CULL_FACE);

	/* Deferred render pass */
	gBuffer.bind();
	gBuffer.bindTextures();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	
    deferredShader.bind();

    /* Set projection and view matrices */
    glUniformMatrix4fv(deferredShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(deferredShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    // static GLuint ubo = 0;
    // if (ubo == 0) {
    //     glGenBuffers(1, &ubo);
    //     glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    //     glBufferData(GL_UNIFORM_BUFFER, 128, NULL, GL_STATIC_DRAW);
    //     glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    //     glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // }
    // glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    // glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(projection));
    // glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(view));
    // glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // GLuint index = glGetUniformBlockIndex(deferredShader.getHandle(), "Matrices");
    // glUniformBlockBinding(deferredShader.getHandle(), index, 0);

    /* Set eye */
    glUniform3fv(deferredShader.uniformLocation("eye"), 1, glm::value_ptr(eye));

	for (GameObject *g : world.getRenderables(projection, view)) {
		Transform *t = g->getComponent<Transform>();
		Mesh *mesh = g->getComponent<Mesh>();
		// Material *m = g->getComponent<Material>();
		Heightmap *hmap = g->getComponent<Heightmap>();
		Texture *texture = g->getComponent<Texture>();

		/* Set material properties */
		// if (m == nullptr) {
		// 	m = defaultMaterial;
		// }
		// m->setUniforms(deferredShader);

		/* Set any textures */
		if (texture == nullptr) {
			/* TODO: not sure if default works */
            defaultTexture.bind();
		}
        else {
            texture->bind();
        }
		glUniform1i(deferredShader.uniformLocation("texture0"), 0);

		/* Set model matrix */
		glm::mat4 model = t->getMatrix();
		glUniformMatrix4fv(deferredShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));

		/* Draw the object */
		if (mesh)
			mesh->draw();
		else if (hmap) {
			hmap->draw();
		}

	}
    deferredShader.unbind();
	gBuffer.unbindTextures();
	gBuffer.unbind();

	glEnable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* Draw to textured quad */
	{
		glm::mat4 p, v, m;

		quadShader.bind();
		glUniformMatrix4fv(quadShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(p));
		glUniformMatrix4fv(quadShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(quadShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(m));

		gBuffer.bindTextures();
		glUniform1i(quadShader.uniformLocation("gPosition"), 0);
		glUniform1i(quadShader.uniformLocation("gNormal"), 1);
		glUniform1i(quadShader.uniformLocation("gAlbedoSpecular"), 2);

		/* Set eye */
		glUniform3fv(quadShader.uniformLocation("eye"), 1, glm::value_ptr(eye));

		/* Set main and point lights */
		world.setLightUBO(quadShader);

		GLQuad::draw();
		gBuffer.unbindTextures();
		quadShader.unbind();
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.getHandle());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint DeferredRenderer::getPosition() const {
    return gBuffer.getTexture(0);
}

GLuint DeferredRenderer::getNormal() const {
    return gBuffer.getTexture(1);
}

GLuint DeferredRenderer::getAlbedoSpecular() const {
    return gBuffer.getTexture(2);
}