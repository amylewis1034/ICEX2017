#include <glm/glm.hpp>
#include <string>

#include "Shader.hpp"
#include <Graphics/GLShaderProgram.hpp>
#include <icex_common.hpp>

Shader::Shader() :
	vertPath(SHADER_PATH "simple.vert"),
	fragPath(SHADER_PATH "simple.frag")
	{
		program.linkProgram(this->vertPath, this->fragPath);
	}
Shader::Shader(std::string vertPath, std::string fragPath) :
	vertPath(vertPath),
	fragPath(fragPath)
	{
		program.linkProgram(this->vertPath, this->fragPath);
	}

Shader::Shader(std::string vertPath, std::string fragPath, std::string geomPath) :
	vertPath(vertPath),
	fragPath(fragPath),
	geomPath(geomPath)
{
	program.linkProgram(vertPath, fragPath, geomPath);
}

Shader::~Shader() {}

void Shader::init() {}
void Shader::update(float dt) {}

void Shader::setVert(std::string vertPath) {
	this->vertPath = vertPath;
}
void Shader::setFrag(std::string fragPath) {
	this->fragPath = fragPath;
}

void Shader::linkProgram() {
	program.linkProgram(this->vertPath, this->fragPath);
}

std::string Shader::getVert() {
	return this->vertPath;
}
std::string Shader::getFrag() {
	return this->fragPath;
}

const GLShaderProgram &Shader::getProgram() const {
	return this->program;
}