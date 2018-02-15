#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>

#include <cassert>

#include "PlayerInput.hpp"
#include "Transform.hpp"
#include <GameObject.hpp>
#include <Input/Keyboard.hpp>
#include <Input/Mouse.hpp>

PlayerInput::PlayerInput() :
    speed(10.0f),
    sensitivity(0.0005f),
	heightLock(true)
    {}

PlayerInput::PlayerInput(float speed, float sensitivity) :
	speed(speed),
	sensitivity(sensitivity),
	heightLock(true)
	{}

PlayerInput::~PlayerInput() {}

void PlayerInput::init() {
    transform = gameobject->getComponent<Transform>();
    assert(transform != nullptr);
}

void PlayerInput::update(float dt) {
    float move_speed = dt * speed;
    glm::vec3 delta = glm::vec3(0.0f);

    if (Keyboard::getKeyDown(GLFW_KEY_W)) {
        delta += move_speed * transform->getForward();
    }
    if (Keyboard::getKeyDown(GLFW_KEY_S)) {
        delta -= move_speed * transform->getForward();
    }
    if (Keyboard::getKeyDown(GLFW_KEY_A)) {
		delta -= move_speed * transform->getRight();
    }
    if (Keyboard::getKeyDown(GLFW_KEY_D)) {
        delta += move_speed * transform->getRight();
    }

	if (heightLock) {
		delta.y = 0.0f;
	}

    transform->translate(delta);

    float xoffset = (float) Mouse::getDeltaX() * sensitivity;
    float yoffset = (float) Mouse::getDeltaY() * sensitivity;

    static glm::vec3 euler = transform->getEulerAngles();

    euler.x = glm::clamp(euler.x + yoffset, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);
    euler.y -= xoffset;

    transform->setRotation(glm::vec3(euler.x, euler.y, 0.0f));
}

float PlayerInput::getSpeed() const {
	return speed;
}

float PlayerInput::getSensitivity() const {
	return sensitivity;
}

void PlayerInput::toggleHeightLock() {
	heightLock = !heightLock;
}

void PlayerInput::setSpeed(float speed) {
	this->speed = speed;
}

void PlayerInput::setSensitivity(float sensitivity) {
	this->sensitivity = sensitivity;
}