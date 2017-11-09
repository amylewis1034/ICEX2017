#ifndef PROJECTIVE_TEXTURE_HPP
#define PROJECTIVE_TEXTURE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include "Component.hpp"
#include <Graphics/GLTexture.hpp>

class ProjectiveTexture : public Component {
public:
    ProjectiveTexture(const glm::vec3 &position, const glm::vec3 &target, const std::string &textureName)
        : position(position), target(target), textureName(textureName)
    {}

    void init() {
        texture.loadTexture(
            textureName,
            GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER,
            GL_LINEAR, GL_LINEAR
        );
    }
    void update(float dt) {}

    glm::mat4 getProjectorMatrix() const {
        static glm::vec3 projUp {0.0f, 1.0f, 0.0f};
        static glm::mat4 projProj = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);
        static glm::mat4 projScaleTrans = glm::translate(glm::vec3(0.5f)) * glm::scale(glm::vec3(0.5f));

        glm::mat4 projView = glm::lookAt(position, target, projUp);
        glm::mat4 m = projScaleTrans * projProj * projView;

        return m;
    }

    glm::vec3 position, target;
    std::string textureName;
    GLTexture texture;
};

#endif
