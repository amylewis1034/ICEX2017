#ifndef PRMINPUT_HPP
#define PRMINPUT_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <Graphics/GLShaderProgram.hpp>
#include <Graphics/GLVertexArrayObject.hpp>
#include <Graphics/GLBuffer.hpp>

#include "Component.hpp"
#include "Transform.hpp"

class PRMInput : public Component {
public:
    PRMInput(const std::string pathfile);
	PRMInput(float speed, const std::string pathfile);
    ~PRMInput();

    void init();
    void update(float dt);

	float getSpeed() const;
	void setSpeed(float speed);
    void setPathfile(const std::string pathfile);

    void postrender(const glm::mat4 &projection, const glm::mat4 &view);
private:
    void initCamPath();
    void setCamPos6dof(const glm::vec3 pos, const glm::vec3 dir);

    Transform *transform;
    float speed;

    int splineNum;

    std::string pathfile;
    int pathLength;
    glm::vec3 camPos, camDir;
    std::vector<glm::vec3> camPosVec, camDirVec;
    GLShaderProgram simpleShader;

    // VAO and VBO for circular path
    GLVertexArrayObject pathsVertArrayObj;
    GLBuffer pathsVertBufObj;
};

#endif