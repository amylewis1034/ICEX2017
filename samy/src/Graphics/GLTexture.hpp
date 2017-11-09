#ifndef GLTEXTURE_HPP
#define GLTEXTURE_HPP

#include <GL/glew.h>
#include <string>

class GLTexture {
public:
    GLTexture();
    GLTexture(const std::string &texname);
    GLTexture(const std::string &texname, GLenum target);
    ~GLTexture();

    void loadTexture(const std::string &texname,
            GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT,
            GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint magFilter = GL_LINEAR);
    void setTarget(GLenum target);
    void bind() const;
    void unbind() const;
    
private:
    GLuint handle, target;

    static GLuint createTexture2D(const std::string &texname,
            GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT,
            GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint magFilter = GL_LINEAR);
};

#endif
