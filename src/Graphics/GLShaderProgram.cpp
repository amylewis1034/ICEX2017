#include <GL/glew.h>
#include <string>
#include <iostream>
#include "GLShaderProgram.hpp"
#include "GLShader.hpp"

GLShaderProgram::GLShaderProgram() :
    handle(0)
    {}

GLShaderProgram::GLShaderProgram(std::string vertSource, std::string fragSource) {
    linkProgram(vertSource, fragSource);
}

GLShaderProgram::GLShaderProgram(std::string vertSource, std::string fragSource, std::string geomSource) {
	linkProgram(vertSource, fragSource, geomSource);
}

GLShaderProgram::~GLShaderProgram() {
    glDeleteProgram(handle);
}

void GLShaderProgram::linkProgram(std::string vertSource, std::string fragSource) {
    this->handle = createProgram(vertSource, fragSource);
}

void GLShaderProgram::linkProgram(std::string vertSource, std::string fragSource, std::string geomSource) {
	GLint success;
	GLchar infoLog[512];

	GLShader vertexShader{ GL_VERTEX_SHADER, vertSource };
	GLShader fragmentShader{ GL_FRAGMENT_SHADER, fragSource };
	GLShader geometryShader{ GL_GEOMETRY_SHADER, geomSource };

	GLuint program;
	program = glCreateProgram();

	glAttachShader(program, vertexShader.getHandle());
	glAttachShader(program, fragmentShader.getHandle());
	glAttachShader(program, geometryShader.getHandle());

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cout
			<< "ERROR::SHADER_PROGRAM::COMPILATION_FAILED\n"
			<< infoLog
			<< std::endl;
	}

	// glDeleteShader(vertexShader);
	// glDeleteShader(fragmentShader);

	this->handle = program;
}

GLuint GLShaderProgram::getHandle() const {
    return handle;
}

void GLShaderProgram::bind() const {
    glUseProgram(handle);
}

void GLShaderProgram::unbind() const {
    glUseProgram(0);
}

GLint GLShaderProgram::uniformLocation(const GLchar *name) {
    return glGetUniformLocation(handle, name);
}

GLuint GLShaderProgram::createProgram(std::string vertSource, std::string fragSource) {
    GLint success;
    GLchar infoLog[512];
    
    GLShader vertexShader {GL_VERTEX_SHADER, vertSource};
    GLShader fragmentShader {GL_FRAGMENT_SHADER, fragSource};

    GLuint program;
    program = glCreateProgram();

    glAttachShader(program, vertexShader.getHandle());
    glAttachShader(program, fragmentShader.getHandle());
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout
            << "ERROR::SHADER_PROGRAM::COMPILATION_FAILED\n"
            << infoLog
            << std::endl;
    }

    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);

    return program;
}
