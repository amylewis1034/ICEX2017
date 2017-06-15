#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Skybox.hpp"
#include <icex_common.hpp>

Skybox::Skybox(const std::vector<std::string> &texnames) {
    skybox.load(texnames);
    shader.linkProgram(SHADER_PATH "skybox.vert", SHADER_PATH "skybox.frag");
    cube = new Mesh(RESOURCE_PATH "objs/cube.obj");
}

Skybox::~Skybox() {
    delete cube;
}

void Skybox::init() {}

void Skybox::update(float dt) {}

void Skybox::draw(const glm::mat4 &projection, const glm::mat4 &view) {
    glDepthFunc(GL_LEQUAL);

    shader.bind();
    skybox.bind();

    glUniformMatrix4fv(shader.uniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(shader.uniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

    cube->draw();

    skybox.unbind();
    shader.unbind();

    glDepthFunc(GL_LESS);
}