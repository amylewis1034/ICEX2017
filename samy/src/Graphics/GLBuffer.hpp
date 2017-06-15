#ifndef GLBUFFER_HPP
#define GLBUFFER_HPP

#include <GL/glew.h>

class GLBuffer {
public:
    GLBuffer();
    ~GLBuffer();
    
    void loadData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);

    void bind() const;
    void unbind() const;
private:
    GLuint handle;
    GLenum target;
};

#endif