#ifndef GLTEXTURE_HPP
#define GLTEXTURE_HPP

#include <GL/glew.h>
#include <string>

class GLTexture {
public:
    GLTexture();
    GLTexture(std::string texname);
    GLTexture(std::string texname, GLenum target);
    ~GLTexture();

    void loadTexture(std::string texname);
    void setTarget(GLenum target);
    void bind() const;
    void unbind() const;
    
private:
    GLuint handle, target;

    static GLuint createTexture2D(std::string texname);
};

#endif