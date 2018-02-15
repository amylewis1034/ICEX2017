#include <GameObject.hpp>
#include "Component.hpp"

Component::Component() {

}

Component::~Component() {

}

void Component::init() {

}

void Component::update(float dt) {

}

void Component::postrender(const glm::mat4 &projection, const glm::mat4 &view) {
    
}

void Component::setGameObject(GameObject *gameobject) {
    this->gameobject = gameobject;
}