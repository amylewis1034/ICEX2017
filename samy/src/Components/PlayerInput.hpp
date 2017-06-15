#ifndef PLAYERINPUT_HPP
#define PLAYERINPUT_HPP

#include "Component.hpp"
#include "Transform.hpp"

class PlayerInput : public Component {
public:
    PlayerInput();
	PlayerInput(float speed, float sensitivity);
    ~PlayerInput();

    void init();
    void update(float dt);

	float getSpeed() const;
	float getSensitivity() const;

	void toggleHeightLock();
	void setSpeed(float speed);
	void setSensitivity(float sensitivity);

	bool heightLock;
private:
    Transform *transform;
    float speed, sensitivity;
};

#endif