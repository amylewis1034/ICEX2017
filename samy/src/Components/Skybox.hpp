#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include "Component.hpp"
#include "Mesh.hpp"
#include <Graphics/GLCubemap.hpp>
#include <Graphics/GLShaderProgram.hpp>
#include <vector>
#include <string>

class Skybox : public Component {
public:
    Skybox(const std::vector<std::string> &texnames);
    ~Skybox();

    void init();
    void update(float dt);

    void draw(const glm::mat4 &projection, const glm::mat4 &view);

private:
    GLCubemap skybox;
    GLShaderProgram shader;
    Mesh *cube;
};

#endif