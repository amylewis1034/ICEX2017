#include <vector>

#include <GameObject.hpp>
#include <Components/Component.hpp>
#include <Components/Transform.hpp>

GameObject::GameObject() {}

GameObject::GameObject(std::string gameObjectTag) {
    tag = gameObjectTag;
}

GameObject::~GameObject() {
    for (Component *c : components) {
        //delete c;
    }
}

void GameObject::addComponent(Component *component) {
    components.push_back(component);
}

void GameObject::init() {
    for (Component *c : components) {
        c->setGameObject(this);
        c->init();
    }

    /* Ensure every GameObject contains a Transform component */
    assert(this->hasComponent<Transform>());
}

void GameObject::update(float dt) {
    for (Component *c : components) {
        c->update(dt);
    }
}

void GameObject::postrender() {
    for (Component *c : components) {
        c->postrender();
    }
}

void GameObject::setTag(std::string newTag) {
    this->tag = newTag;
}

const std::string &GameObject::getTag() const {
    return tag;
}