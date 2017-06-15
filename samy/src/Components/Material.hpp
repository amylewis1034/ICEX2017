#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <glm/glm.hpp>
#include "Component.hpp"
#include <Graphics/GLShaderProgram.hpp>

class Material : public Component {
public:
    Material();
    Material(glm::vec3 ka, glm::vec3 kd, glm::vec3 ks, float shine, float alpha);
    virtual ~Material();

    virtual void init();
    virtual void update(float dt);

	void setUniforms(GLShaderProgram &program);

 private:
    glm::vec3 ka, kd, ks;
    float shine, alpha;
};

#endif