#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include "Component.hpp"

class Camera : public Component {
public:
    Camera();
    Camera(float fov, float near, float far, bool isFirstPerson);

    void init();
    void update(float dt);

    const glm::mat4 &getProjection() const;
    const glm::mat4 &getView() const;
	const glm::vec3 &getOffset() const;
    float getSpringFactor() const;
    
	void setOffset(const glm::vec3 &offset);
    void setFirstPerson();
    void setSpringFactor(float k);

    const float getNear() const { return near; }
    const float getFar() const { return far; }

private:
    float fov, aspect, near, far;
    glm::mat4 projection;
    glm::mat4 view;

    float k;
	glm::vec3 offset;

    glm::mat4 calcView(float dt);
};

#endif