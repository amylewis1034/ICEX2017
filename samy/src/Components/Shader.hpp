#ifndef SHADER_HPP
#define SHADER_HPP

#include <glm/glm.hpp>
#include <string>
#include <map>

#include "Component.hpp"
#include <Graphics/GLShaderProgram.hpp>

class Shader : public Component {
public:
	Shader();
	Shader(std::string vertPath, std::string fragPath);
	Shader(std::string vertPath, std::string fragPath, std::string geomPath);
	~Shader();

	void init();
	void update(float dt);

	void setVert(std::string vertPath);
	void setFrag(std::string fragPath);
	void linkProgram();

	std::string getVert();
	std::string getFrag();

	const GLShaderProgram &getProgram() const;

private:
	std::string vertPath;
	std::string fragPath;
	std::string geomPath;

	GLShaderProgram program;

};

#endif