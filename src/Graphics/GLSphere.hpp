#ifndef GLSPHERE_HPP
#define GLSPHERE_HPP

#include <GL/glew.h>

class Mesh;

class GLSphere {
public:
	static void init();
	static void draw();
private:
	static Mesh *mesh;
};

#endif