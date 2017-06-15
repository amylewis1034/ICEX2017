#ifndef GLSHADER_HPP
#define GLSHADER_HPP

#include <GL/glew.h>
#include <string>

class GLShader {
public:
    GLShader();
    GLShader(GLenum shaderType, std::string shaderSource);
    ~GLShader();

    void loadShader(GLenum shaderType, std::string shaderSource);
    GLuint getHandle() const;
    
private:
    GLuint handle;

    static GLuint createShader(GLenum shaderType, std::string filename);
    static std::string readText(std::string filename);
};

#endif