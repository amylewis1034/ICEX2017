#ifndef FIRSTPERSONCAMERA_HPP
#define FIRSTPERSONCAMERA_HPP

#include <glm/glm.hpp>
#include "Component.hpp"

class FirstPersonCamera : public Component {
public:
    FirstPersonCamera();
    FirstPersonCamera(float fov, float aspect, float near, float far);

    void init();
    void update(float dt);

    const glm::mat4 &getProjection() const;
    const glm::mat4 &getView() const;
private:
    float fov, aspect, near, far;
    glm::mat4 projection;
    glm::mat4 view;

    glm::mat4 calcView();
};

#endif