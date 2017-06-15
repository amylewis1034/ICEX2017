/*
 * https://docs.unity3d.com/ScriptReference/Transform.html
 * https://glm.g-truc.net/0.9.4/api/a00153.html
 *
 */

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Component.hpp"

class Transform : public Component {
public:
    Transform();
    Transform(glm::vec3 position, glm::vec3 scale);
    ~Transform();

    void rotate(float angle, const glm::vec3 &axis);
    void rotate(const glm::quat &rotation);
    void scale(const glm::vec3 &scale);
    void translate(const glm::vec3 &offset);

    void init();
    void update(float dt);

    const glm::vec3 &getPosition() const;
    glm::vec3 getEulerAngles() const;
    const glm::vec3 &getScale() const;
    const glm::mat4 &getMatrix();

    const glm::vec3 &getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;

    void setPosition(const glm::vec3 &position);
	void setY(float y);
    void setRotation(const glm::vec3 &euler);
    void setScale(const glm::vec3 &scale);
    void setForward(const glm::vec3 &forward);

private:
	glm::vec3 localPosition;
	glm::quat localRotation;
	glm::vec3 localScale;

    glm::vec3 forward;

    glm::mat4 currMatrix;
    bool isDirty;

    Transform *parent;   

    void computeMatrix(); 
};

#endif