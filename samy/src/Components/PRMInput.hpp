#ifndef PRMINPUT_HPP
#define PRMINPUT_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Component.hpp"
#include "Transform.hpp"

class PRMInput : public Component {
public:
    PRMInput(const std::string pathfile);
	PRMInput(float speed, const std::string pathfile, bool generatePath = false);
    ~PRMInput();

    void init();
    void update(float dt);

	float getSpeed() const;

	void setSpeed(float speed);
private:
    void initCamPath();
    void setCamPos6dof(const glm::vec3 pos, const glm::vec3 dir);

    Transform *transform;
    float speed;

    int totalPathLen;
    int splineNum;
    bool generatePath;

    std::string pathfile;
    int pathLength;
    glm::vec3 camPos, camDir;
    std::vector<glm::vec3> camPosVec, camDirVec;
};

#endif