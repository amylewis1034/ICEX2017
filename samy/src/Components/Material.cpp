#include <glm/glm.hpp>
#include "Material.hpp"

Material::Material() :
    ka(0.2f), kd(0.5f), ks(0.8f), shine(20.0f), alpha(1.0f)
    {}

Material::Material(glm::vec3 ka, glm::vec3 kd, glm::vec3 ks, float shine, float alpha) :
    ka(ka), kd(kd), ks(ks), shine(shine), alpha(alpha)
    {}

Material::~Material() {}

void Material::init() {}
void Material::update(float dt) {}

void Material::setUniforms(GLShaderProgram &program) {
	glUniform3f(program.uniformLocation("matAmb"), this->ka.x, this->ka.y, this->ka.z);
	glUniform3f(program.uniformLocation("matDif"), this->kd.x, this->kd.y, this->kd.z);
	glUniform3f(program.uniformLocation("matSpec"), this->ks.x, this->ks.y, this->ks.z);
	glUniform1f(program.uniformLocation("matShine"), this->shine);
	glUniform1f(program.uniformLocation("matAlpha"), this->alpha);
}