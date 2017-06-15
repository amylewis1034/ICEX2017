#ifndef COLLIDER_HPP
#define COLLIDER_HPP

#include <GameObject.hpp>
#include "Component.hpp"

#include <glm/glm.hpp>

class Mesh;

class Collider : public Component {
public:
	Collider();
	Collider(glm::vec3 min, glm::vec3 max);
	Collider(const Mesh &mesh);
	~Collider();

	void init();
    void update(float dt);

    void transform(const glm::mat4 &ctm);
    bool hit(const Collider &other);

    void setMinOrig(glm::vec3 min);
    void setMaxOrig(glm::vec3 max);

// private:
	glm::vec3 min, max;
	glm::vec3 minOrig, maxOrig;
	bool isDirty;
	bool collided;
};

#endif