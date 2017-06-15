#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "FirstPersonCamera.hpp"
#include "Transform.hpp"
#include <GameObject.hpp>

FirstPersonCamera::FirstPersonCamera() :
    fov(45.0f),
    aspect(4.0f / 3.0f),
    near(0.1f),
    far(100.0f)
    {}

FirstPersonCamera::FirstPersonCamera(float fov, float aspect, float near, float far) :
    fov(fov),
    aspect(aspect),
    near(near),
    far(far)
    {}

void FirstPersonCamera::init() {    
    this->projection = glm::perspective(fov, aspect, near, far);
    this->view = this->calcView();
}

void FirstPersonCamera::update(float dt) {
    this->view = this->calcView();
}

const glm::mat4 &FirstPersonCamera::getProjection() const {
    return this->projection;
}

const glm::mat4 &FirstPersonCamera::getView() const {
    return view;
}

glm::mat4 FirstPersonCamera::calcView() {
    Transform *transform = gameobject->getComponent<Transform>();
    return glm::lookAt(transform->getPosition(), transform->getPosition() + transform->getForward(), transform->getUp());
}