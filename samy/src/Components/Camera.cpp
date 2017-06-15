#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.hpp"
#include "Transform.hpp"
#include <GameObject.hpp>
#include <icex_common.hpp>
//static glm::vec3 offset {0.0f, 0.5f, 1.0f};

Camera::Camera() :
    fov(45.0f),
    aspect((float)WIDTH / HEIGHT),
    near(0.1f),
    far(250.0f),
    k(10.0f),
	offset(0.0f, 0.5f, 1.0f)
    {}

Camera::Camera(float fov, float aspect, float near, float far) :
    fov(fov),
    aspect(aspect),
    near(near),
    far(far),
    k(10.0f),
	offset(0.0f, 0.5f, 1.0f)
    {}

void Camera::init() {    
    this->projection = glm::perspective(fov, aspect, near, far);
    this->view = this->calcView(0.0f);
}

void Camera::update(float dt) {
    this->view = this->calcView(dt);
}

const glm::mat4 &Camera::getProjection() const {
    
    return this->projection;
}

const glm::mat4 &Camera::getView() const {
    return view;
}

float Camera::getSpringFactor() const {
    return k;
}

const glm::vec3 &Camera::getOffset() const {
	return offset;
}

void Camera::setOffset(const glm::vec3 &offset) {
	this->offset = offset;
}

void Camera::setFirstPerson() {
    this->offset = glm::vec3(0.0f);
    this->k = 1e9;
}

void Camera::setSpringFactor(float k) {
    this->k = k;
}

glm::mat4 Camera::calcView(float dt) {
    Transform *transform = gameobject->getComponent<Transform>();

    /* Should try using quaternion slerp too */
    // glm::vec3 nexteye = transform->getPosition() + offset * -transform->getForward();

    glm::vec3 nexteye =
        transform->getPosition() +
        offset.x * transform->getRight() +
        offset.y * transform->getUp() +
        offset.z * -transform->getForward();

    static glm::vec3 eye = nexteye;
    eye = glm::mix(eye, nexteye, glm::clamp(k * dt, 0.0f, 1.0f));

    return glm::lookAt(eye, transform->getPosition() + transform->getForward(), transform->getUp());
    // return glm::lookAt(transform->getPosition(), transform->getPosition() + transform->getForward(), transform->getUp());
}