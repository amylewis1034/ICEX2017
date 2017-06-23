#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject;

class Component {
public:
    Component();
    virtual ~Component();

    virtual void init();
    virtual void update(float dt);
    virtual void postrender(const glm::mat4 &projection, const glm::mat4 &view);

    void setGameObject(GameObject *gameobject);

protected:
    GameObject *gameobject;
};

#endif