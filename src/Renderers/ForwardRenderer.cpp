#include "ForwardRenderer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <World.hpp>
#include <Graphics/GLTexture.hpp>
#include <Graphics/GLShaderProgram.hpp>
#include <Components/Transform.hpp>
#include <Components/Mesh.hpp>
#include <Components/Heightmap.hpp>
#include <Components/Texture.hpp>
#include <GLFW/glfw3.h>
#include <icex_common.hpp>

ForwardRenderer::ForwardRenderer() :
    defaultMaterial(),
    defaultShader(SHADER_PATH "phong.vert", SHADER_PATH "phong.frag"),
    program(defaultShader.getProgram())
    {}

ForwardRenderer::~ForwardRenderer() {}

void ForwardRenderer::render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
	
    for (GameObject *g : world.getRenderables(projection, view)) {
		Transform *t = g->getComponent<Transform>();
		Mesh *mesh = g->getComponent<Mesh>();
		Shader *s = g->getComponent<Shader>();
		Material *m = g->getComponent<Material>();
		Heightmap *hmap = g->getComponent<Heightmap>();
		Texture *texture = g->getComponent<Texture>();

		if (s != nullptr) {
			program = s->getProgram();
		}
		else {
			program = defaultShader.getProgram();
		}

		program.bind();

		/* Set projection and view matrices */
		glUniformMatrix4fv(program.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(program.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

		/* Set eye */
		glUniform3fv(program.uniformLocation("eye"), 1, glm::value_ptr(eye));

		/* Set main and point lights */
		world.setLightUniforms(program);

		/* Set material properties */
		if (m == nullptr) {
			m = &defaultMaterial;
		}
		m->setUniforms(program);

		/* Set any textures */
		if (texture != nullptr) {
			glUniform1i(program.uniformLocation("textureName"), 0);
			texture->bind();
		}
		
		/* Set time */
        glUniform1f(program.uniformLocation("time"), 0.4f * (float)glfwGetTime());

		/* Set model matrix */
		glm::mat4 model = t->getMatrix();
		glUniformMatrix4fv(program.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));

		/* Draw the object */
		if (mesh)
			mesh->draw();
		else if (hmap) {
			hmap->draw();
		}

		program.unbind();
	}
}