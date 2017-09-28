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
#include "PRM/Utilities.h"

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

    causticShader.linkProgram(SHADER_PATH "caustics.vert", SHADER_PATH "caustics.frag");

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
    glCullFace(GL_FRONT);

    shadowmapShader.bind();

    const glm::vec3 &lightPos = world.getMainlightPosition();
    const float lz_near = 0.1f, lz_far = 75.0f, l_boundary = 50.0f;
    glm::mat4 lp = glm::ortho(-l_boundary, l_boundary, -l_boundary, l_boundary, lz_near, lz_far);
    glm::mat4 lv = glm::lookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0.0f, 0.0f, -1.0f));
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
		Heightmap *hmap = g->getComponent<Heightmap>();
		Texture *texture = g->getComponent<Texture>();
        WaterMesh *water = g->getComponent<WaterMesh>();

		/* Set any textures */
		if (texture == nullptr) {
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
        else if (water) {
            //water->draw();
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
    glUniform1i(dirlightShader.uniformLocation("genCombo"), world.getRenderSetting().genCombo);

    GLQuad::draw();

    gBuffer.unbindTextures();
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    dirlightShader.unbind();

    glEnable(GL_DEPTH_TEST);

    glDisable(GL_BLEND);

    if (GLEW_KHR_debug) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Caustics");
    }

     glCullFace(GL_FRONT);
    //glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GameObject *watermesh = world.getGameObjectWithComponent<WaterMesh>();
    if (watermesh) {
        WaterMesh *water = watermesh->getComponent<WaterMesh>();
        Transform *water_transform = watermesh->getComponent<Transform>();

        causticShader.bind();
        gBuffer.bindTextures();
        glUniform1i(causticShader.uniformLocation("world_positions"), 0);
        
        GLuint block_index;
        block_index = glGetUniformBlockIndex(causticShader.getHandle(), "Caustics");
        glUniformBlockBinding(causticShader.getHandle(), block_index, 0);

        glUniformMatrix4fv(causticShader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(causticShader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
        
        glUniform3fv(causticShader.uniformLocation("eye"), 1, glm::value_ptr(eye));

        water->draw_caustics();
        // cube->draw_instanced(16 * 16);
        // for (int i = 0; i < water->indices.size() - 2; i++) {
        //     // i = ((int) (4 * glfwGetTime())) % (water->indices.size() - 2);
        //     glUniform1uiv(causticShader.uniformLocation("index"), 3, &water->indices[i]);

        //     glm::mat4 model, t, s;
        //     BoundingBox &bbox = water->bboxes[i];
        //     t = glm::translate(t, bbox.center + water_transform->getPosition());
        //     s = glm::scale(s, bbox.scale * water_transform->getScale());
        //     model = t * s;         
        //     glUniformMatrix4fv(causticShader.uniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
            

        //     // std::cout << "=================" << std::endl;
        //     // unsigned *indices = &water->indices[i];
        //     // std::cout << indices[0] << " " << indices[1] << " " << indices[2] << std::endl;
        //     // glm::vec3 v[3] = {
        //     //     water->vertices[indices[0]],
        //     //     water->vertices[indices[1]],
        //     //     water->vertices[indices[2]]
        //     // };
        //     // glm::vec3 n[3] = {
        //     //     water->normals[indices[0]],
        //     //     water->normals[indices[1]],
        //     //     water->normals[indices[2]]
        //     // };
        //     // glm::vec3 r[3] = {
        //     //     water->refracted_rays[indices[0]],
        //     //     water->refracted_rays[indices[1]],
        //     //     water->refracted_rays[indices[2]]
        //     // };
        //     // glm::vec3 basis_x = glm::normalize(v[1] - v[0]);
        //     // glm::vec3 basis_y = glm::normalize(
        //     //     glm::cross(
        //     //         v[1] - v[0], v[2] - v[0]
        //     //     )
        //     // );
        //     // // glm::vec3 basis_y = glm::normalize(n[0] + n[1] + n[2]);
        //     // glm::vec3 basis_z = glm::cross(basis_x, basis_y);
        //     // glm::mat3 cob {basis_x, basis_y, basis_z};
        //     // cob = glm::transpose(cob);

        //     // std::cout << glm::to_string(basis_x) << std::endl;
        //     // std::cout << glm::to_string(basis_y) << std::endl;
        //     // std::cout << glm::to_string(basis_z) << std::endl;

        //     // for (int j = 0; j < 3; j++) {
        //     //     std::cout << "vertex[" << j << "]: " <<
        //     //         glm::to_string(v[j]) << " " <<
        //     //         glm::to_string(n[j]) << " " <<
        //     //         glm::to_string(r[j]) << " " << std::endl;
        //     // }
        //     // v[0] = cob * v[0];
        //     // v[1] = cob * v[1];
        //     // v[2] = cob * v[2];
        //     // r[0] = cob * r[0];
        //     // r[1] = cob * r[1];
        //     // r[2] = cob * r[2];
        //     // r[0] /= r[0].y;
        //     // r[1] /= r[1].y;
        //     // r[2] /= r[2].y;
        //     // for (int j = 0; j < 3; j++) {
        //     //     std::cout << "vertex[" << j << "]: " <<
        //     //         glm::to_string(v[j]) << " " <<
        //     //         glm::to_string(n[j]) << " " <<
        //     //         glm::to_string(r[j]) << " " << std::endl;
        //     // }

        //     cube->draw();
        //     // break;
        // }

        gBuffer.unbindTextures();
        causticShader.unbind();

        glDisable(GL_BLEND);
    }
    
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    if (GLEW_KHR_debug) {
        glPopDebugGroup();
    }

    glDepthMask(GL_TRUE);    

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
