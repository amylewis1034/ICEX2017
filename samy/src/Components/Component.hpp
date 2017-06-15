#ifndef COMPONENT_HPP
#define COMPONENT_HPP

class GameObject;

class Component {
public:
    Component();
    virtual ~Component();

    virtual void init();
    virtual void update(float dt);

    void setGameObject(GameObject *gameobject);

protected:
    GameObject *gameobject;
};

#endif