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

void Component::setGameObject(GameObject *gameobject) {
    this->gameobject = gameobject;
}