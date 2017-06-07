#ifndef __Texture__
#define __Texture__

#define GLEW_STATIC
#include <GL/glew.h>

#include <string>

class Texture
{
public:
	Texture();
	virtual ~Texture();
	void setFilename(const std::string &f) { filename = f; }
	void init();
	// void setUnit(GLint u) { unit = u; }
	void bind(GLint handle, GLint unit);
	void unbind(GLint unit);
    void setWrapModes(GLint wrapS, GLint wrapT); // Must be called after init()

private:
	std::string filename;
	int width;
	int height;
	GLuint tid;
	// GLint unit;
};

#endif
