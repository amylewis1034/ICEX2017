#ifndef GLHELPER_HPP
#define GLHELPER_HPP

#include <GL/glew.h>

class GLHelper {
public:
    static void printGLInfo();
    static void printGLExtensions();
    static void registerDebugOutputCallback();
    static void printUniformInfo(GLuint program);
};

#endif