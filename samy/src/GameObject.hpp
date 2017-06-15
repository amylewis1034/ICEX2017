#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include <vector>
#include <string>

#include <Components/Component.hpp>

class GameObject {
public:
    GameObject();
    ~GameObject();

    GameObject(std::string gameObjectTag);

    void addComponent(Component *component);

    template <typename T>
    bool hasComponent() const {
        T *component;
        for (Component *c : components) {
            component = dynamic_cast<T *>(c);
            if (component != nullptr) {
                return true;
            }
        }

        return false;
    }
    
    template <typename T>
    T *getComponent() const {
        T *component;
        for (Component *c : components) {
            component = dynamic_cast<T *>(c);
            if (component != nullptr) {
                return component;
            }
        }

        return nullptr;
    }

    void init();
    void update(float dt);
    void setTag(std::string newTag);
    const std::string &getTag() const;

private:
    std::vector<Component *> components;
    GameObject *parent;
    std::string tag;
};

#endif