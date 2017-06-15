#ifndef GLCUBEMAP_HPP
#define GLCUBEMAP_HPP

#include <GL/glew.h>
#include <string>
#include <vector>

class GLCubemap {
public:
    GLCubemap();
    GLCubemap(const std::vector<std::string> &texnames);
    ~GLCubemap();

    void load(const std::vector<std::string> &texnames);
    void setTextureUnit(GLenum texunit);
    void bind() const;
    void unbind() const;
private:
    GLuint handle, texunit;

    static GLuint createTexture2D(std::string texname);
};

#endif