#include "DeferredShadowRenderer.hpp"
#include "PostprocessRenderer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <World.hpp>
#include <Graphics/GLFramebuffer.hpp>
#include <Graphics/GLTexture.hpp>
#include <Graphics/GLShaderProgram.hpp>
#include <Graphics/GLQuad.hpp>
#include <Graphics/GLSphere.hpp>
#include <Components/Transform.hpp>
#include <Components/Mesh.hpp>
#include <Components/Heightmap.hpp>
#include <Components/Texture.hpp>
#include <icex_common.hpp>
#include "GLFW/glfw3.h"

#include <GL/glew.h>

#include <glm/gtx/string_cast.hpp>

extern GLFWwindow *window;

DeferredShadowRenderer::DeferredShadowRenderer() {
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

    // gBuffer.attachRenderbuffer(
    //     GL_DEPTH_ATTACHMENT,
    //     GL_DEPTH_COMPONENT,
    //     width,
    //     height
    // );

    gBuffer.attachTexture(
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_DEPTH24_STENCIL8,
        width,
        height,
        GL_DEPTH_STENCIL,
        GL_UNSIGNED_INT_24_8
    );

    if (gBuffer.getStatus() != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not created successfully" << std::endl;
    }
    gBuffer.unbind();

    lightFBO.bind();
    glm::vec4 borderColor {1.0f};
    lightFBO.attachTexture(
        GL_DEPTH_ATTACHMENT,
        GL_DEPTH_COMPONENT,
        width,
        height,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        GL_LINEAR,
        GL_LINEAR,
        GL_CLAMP_TO_BORDER,
        GL_CLAMP_TO_BORDER,
        &borderColor
    );
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (lightFBO.getStatus() != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Light Framebuffer not created successfully" << std::endl;
    }
    lightFBO.unbind();

    /* Initialize default texture */
    defaultTexture.loadTexture(RESOURCE_PATH "textures/default.png");

    /* Initialize shaders */
    deferredShader.linkProgram(SHADER_PATH "phong.vert", SHADER_PATH "geometry_pass.frag");
    shadowmapShader.linkProgram(SHADER_PATH "phong.vert", SHADER_PATH "empty.frag");
    quadShader.linkProgram(SHADER_PATH "textured_quad.vert", SHADER_PATH "deferred_ubo_shadows.frag");

    dirlightShader.linkProgram(SHADER_PATH "textured_quad.vert", SHADER_PATH "dirlight.frag");
    pointlightShader.linkProgram(SHADER_PATH "lightpass.vert", SHADER_PATH "pointlight.frag");
    lightStencilShader.linkProgram(SHADER_PATH "lightpass.vert", SHADER_PATH "empty.frag");

    causticShader.linkProgram(
        SHADER_PATH "lightz.vert",
        SHADER_PATH "lightz.frag",
        SHADER_PATH "lightz.geom"
    );

    /* Initialize quad and sphere*/
    GLQuad::init();
    GLSphere::init();
}

DeferredShadowRenderer::~DeferredShadowRenderer() {}

void DeferredShadowRenderer::render(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &eye, World &world) {
	if (GLEW_KHR_debug) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Deferred Shading Pass");
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

    /* Shadow map */
    if (GLEW_KHR_debug) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Shadow Map Pass");
    }
    lightFBO.bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);

    shadowmapShader.bind();
    
    const glm::vec3 &lightPos = world.getMainlightPosition();
    glm::mat4 lp = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 20.0f);
    glm::mat4 lv = glm::lookAt(lightPos, glm::vec3(0.0f, 10.0f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(shadowmapShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(lp));
    glUniformMatrix4fv(shadowmapShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(lv));
	for (GameObject *g : world.getRenderables(projection, view)) {
		Transform *t = g->getComponent<Transform>();
		Mesh *mesh = g->getComponent<Mesh>();
		Heightmap *hmap = g->getComponent<Heightmap>();

		/* Set model matrix */
		glm::mat4 model = t->getMatrix();
		glUniformMatrix4fv(shadowmapShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));

		/* Draw the object */
		if (mesh)
			mesh->draw();
		else if (hmap) {
			hmap->draw();
		}
	}
    lightFBO.unbind();
    shadowmapShader.unbind();
    if (GLEW_KHR_debug) {
        glPopDebugGroup();
    }

	/* Geometry pass */
	if (GLEW_KHR_debug) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Geometry Pass");
    }
	gBuffer.bind();
	gBuffer.bindTextures();
	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);
    glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
    glCullFace(GL_BACK);
	
    deferredShader.bind();

    /* Set projection and view matrices */
    glUniformMatrix4fv(deferredShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(deferredShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

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
	if (GLEW_KHR_debug) {
        glPopDebugGroup();
    }

    static const PostprocessRenderer *pr = dynamic_cast<const PostprocessRenderer *>(world.getPostrenderer());
    GLuint nextFBO = 0;
    if (pr != nullptr) {
        nextFBO = pr->getFBO();
    }
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.getHandle());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, nextFBO);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);

    glClear(GL_COLOR_BUFFER_BIT);

    glDepthMask(GL_FALSE);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_STENCIL_TEST);

    // TODO: render lights instanced?
    struct pointlight {
        glm::vec3 pos, color;
        float a, b, c;
    };
    extern std::vector<pointlight> pointlights;

    // Light volume technique http://ogldev.atspace.co.uk/www/tutorial36/tutorial36.html
    // Point lights
    pointlightShader.bind();
    glUniformMatrix4fv(pointlightShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(pointlightShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

    gBuffer.bindTextures();
    glUniform1i(pointlightShader.uniformLocation("gPosition"), 0);
    glUniform1i(pointlightShader.uniformLocation("gNormal"), 1);
    glUniform1i(pointlightShader.uniformLocation("gAlbedoSpecular"), 2);

    const glm::vec2 screenSize = glm::vec2(width, height);
    glUniform2fv(pointlightShader.uniformLocation("screenSize"), 1, glm::value_ptr(screenSize));
    glUniform3fv(pointlightShader.uniformLocation("eye"), 1, glm::value_ptr(eye));

    for (const auto &p : pointlights) {
        glUniform3fv(pointlightShader.uniformLocation("plight.pos"), 1, glm::value_ptr(p.pos));
		glUniform3fv(pointlightShader.uniformLocation("plight.color"), 1, glm::value_ptr(p.color));
		glUniform1f(pointlightShader.uniformLocation("plight.a"), p.a);
		glUniform1f(pointlightShader.uniformLocation("plight.b"), p.b);
		glUniform1f(pointlightShader.uniformLocation("plight.c"), p.c);

        float maxlight = glm::max(glm::max(p.color.r, p.color.g), p.color.b);
        float radius = (-p.b + glm::sqrt(p.b * p.b - 4 * p.c * (p.a - maxlight * 256.0f / 1.0f))) / (2.0f * p.c);

    //     // std::cout << glm::to_string(p.pos) << " " << glm::to_string(p.color);
    //     // printf(" %f %f %f %f\n", p.a, p.b, p.c, radius);

        glm::mat4 trans, scale, pm;
        trans = glm::translate(trans, p.pos);
        scale = glm::scale(scale, glm::vec3(radius));
        pm = trans * scale;
        glUniformMatrix4fv(pointlightShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(pm));

        /* Stencil Pass */
        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilFunc(GL_ALWAYS, 0, 0);
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        lightStencilShader.bind();
        glUniformMatrix4fv(lightStencilShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(lightStencilShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(lightStencilShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(pm));
    
        GLSphere::draw();
        lightStencilShader.unbind();

        /* Lighting Pass */
        glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        pointlightShader.bind();
        
        GLSphere::draw();

        glDisable(GL_BLEND);
        glCullFace(GL_BACK);
    }

    gBuffer.unbindTextures();
    pointlightShader.unbind();

    glDisable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    // Directional light
    glm::mat4 p, v, m;
    dirlightShader.bind();
    glUniformMatrix4fv(dirlightShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(p));
    glUniformMatrix4fv(dirlightShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(v));
    glUniformMatrix4fv(dirlightShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(m));

    gBuffer.bindTextures();
    glUniform1i(dirlightShader.uniformLocation("gPosition"), 0);
    glUniform1i(dirlightShader.uniformLocation("gNormal"), 1);
    glUniform1i(dirlightShader.uniformLocation("gAlbedoSpecular"), 2);

    glUniform3fv(dirlightShader.uniformLocation("eye"), 1, glm::value_ptr(eye));

    const glm::vec3 &dirlightPos = world.getMainlightPosition();
    const glm::vec3 &dirlightColor = world.getMainlightColor();
	glUniform3fv(dirlightShader.uniformLocation("dirlightPos"), 1, glm::value_ptr(dirlightPos));
	glUniform3fv(dirlightShader.uniformLocation("dirlightColor"), 1, glm::value_ptr(dirlightColor));

    glm::mat4 ls = lp * lv;
    glUniformMatrix4fv(dirlightShader.uniformLocation("ls"), 1, GL_FALSE, glm::value_ptr(ls));
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, lightFBO.getTexture(0));
    glUniform1i(dirlightShader.uniformLocation("shadowMap"), 3);

    glUniformMatrix4fv(dirlightShader.uniformLocation("geomView"), 1, GL_FALSE, glm::value_ptr(view));    
    glUniform1i(dirlightShader.uniformLocation("genNormals"), world.getRenderSetting().genNormals) ;
    glUniform1i(dirlightShader.uniformLocation("genThirds"), world.getRenderSetting().genThirds);

    GLQuad::draw();

    gBuffer.unbindTextures();
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    dirlightShader.unbind();

    // Caustics
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    causticShader.bind();

    glUniformMatrix4fv(causticShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(causticShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(causticShader.uniformLocation("lp"), 1, GL_FALSE, glm::value_ptr(lp));
    glUniformMatrix4fv(causticShader.uniformLocation("lv"), 1, GL_FALSE, glm::value_ptr(lv));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lightFBO.getTexture(0));
    glUniform1i(causticShader.uniformLocation("shadowMap"), 0);

    glUniform3fv(causticShader.uniformLocation("lightPos"), 1, glm::value_ptr(lightPos));
    
    float time = glfwGetTime();
    glUniform1f(causticShader.uniformLocation("time"), time);

    static bool first = true;
    static GLVertexArrayObject gridvao;
    if (first) {
        first = false;

        static glm::vec4 grid[200][200];
        for (int i = 0; i < 200; i++) {
            for (int j = 0; j < 200; j++) {
                grid[i][j] = glm::vec4(
                    (i - 100) / 20.0f,
                    (j - 100) / 20.0f,
                    -5,
                    1
                );
            }
        }
        static GLBuffer gridbuf;
        gridbuf.loadData(GL_ARRAY_BUFFER, 200 * 200 * sizeof(glm::vec4), grid, GL_STATIC_DRAW);
        gridvao.addAttribute(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    }

    gridvao.bind();
    glDrawArrays(GL_POINTS, 0, 200 * 200);
    gridvao.unbind();
    causticShader.unbind();

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

	/* Draw to textured quad */
	// {
	// 	quadShader.bind();
	// 	glUniformMatrix4fv(quadShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(p));
	// 	glUniformMatrix4fv(quadShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(v));
	// 	glUniformMatrix4fv(quadShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(m));

	// 	gBuffer.bindTextures();
	// 	glUniform1i(quadShader.uniformLocation("gPosition"), 0);
	// 	glUniform1i(quadShader.uniformLocation("gNormal"), 1);
	// 	glUniform1i(quadShader.uniformLocation("gAlbedoSpecular"), 2);

	// 	/* Set eye */
	// 	glUniform3fv(quadShader.uniformLocation("eye"), 1, glm::value_ptr(eye));

	// 	/* Set main and point lights */
	// 	world.setLightUBO(quadShader);

    //     glm::mat4 ls = lp * lv;
    //     glUniformMatrix4fv(quadShader.uniformLocation("ls"), 1, GL_FALSE, glm::value_ptr(ls));
    //     glActiveTexture(GL_TEXTURE3);
    //     glBindTexture(GL_TEXTURE_2D, lightFBO.getTexture(0));
    //     glUniform1i(quadShader.uniformLocation("shadowMap"), 3);

	// 	GLQuad::draw();
	// 	gBuffer.unbindTextures();
    //     glActiveTexture(GL_TEXTURE3);
    //     glBindTexture(GL_TEXTURE_2D, 0);
	// 	quadShader.unbind();
	// }

    glDepthMask(GL_TRUE);

	// glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.getHandle());
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, nextFBO);
	// glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    // glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);

    glEnable(GL_DEPTH_TEST);

    if (GLEW_KHR_debug) {
        glPopDebugGroup();
    }
}

GLuint DeferredShadowRenderer::getPosition() const {
    return gBuffer.getTexture(0);
}

GLuint DeferredShadowRenderer::getNormal() const {
    return gBuffer.getTexture(1);
}

GLuint DeferredShadowRenderer::getAlbedoSpecular() const {
    return gBuffer.getTexture(2);
}

GLuint DeferredShadowRenderer::getLightDepth() const {
    return lightFBO.getTexture(0);
}