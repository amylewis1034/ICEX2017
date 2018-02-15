#ifndef GLQUAD_HPP
#define GLQUAD_HPP

#include <GL/glew.h>

class GLQuad {
public:
	static void init();
	static void draw();
private:
	static GLuint vao;
};

#endif