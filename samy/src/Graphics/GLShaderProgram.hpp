#ifndef GLSHADERPROGRAM_HPP
#define GLSHADERPROGRAM_HPP

#include <GL/glew.h>
#include <string>

class GLShaderProgram {
public:
    GLShaderProgram();
    GLShaderProgram(std::string vertSource, std::string fragSource);
	GLShaderProgram(std::string vertSource, std::string fragSource, std::string geomSource);
    ~GLShaderProgram();

    void linkProgram(std::string vertSource, std::string fragSource);
	void linkProgram(std::string vertSource, std::string fragSource, std::string geomSource);
    GLuint getHandle() const;

    void bind() const;
    void unbind() const;
    GLint uniformLocation(const GLchar *name);

private:
    GLuint handle;

    static GLuint createProgram(std::string vertSource, std::string fragSource);
};

#endif