#include <GL/glew.h>
#include <string>
#include <iostream>
#include "GLTexture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLTexture::GLTexture() :
    handle(0), target(GL_TEXTURE0)
    {}

GLTexture::GLTexture(const std::string &texname) {
    loadTexture(texname);
    this->target = GL_TEXTURE0;
}

GLTexture::GLTexture(const std::string &texname, GLenum target) {
    loadTexture(texname);
    this->target = target;
}

GLTexture::~GLTexture() {
    glDeleteTextures(1, &handle);
}

void GLTexture::loadTexture(const std::string &texname, GLint wrapS, GLint wrapT, GLint minFilter, GLint magFilter) {
    this->handle = createTexture2D(texname, wrapS, wrapT, minFilter, magFilter);
}

void GLTexture::setTarget(GLenum target) {
    this->target =  target;
}

void GLTexture::bind() const {
    glActiveTexture(target);
    glBindTexture(GL_TEXTURE_2D, handle);
}

void GLTexture::unbind() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint GLTexture::createTexture2D(const std::string &texname, GLint wrapS, GLint wrapT, GLint minFilter, GLint magFilter) {
    int width, height, channels;
    unsigned char *image = stbi_load(texname.c_str(), &width, &height, &channels, 0);
    if (image == NULL) {
        std::cout
            << "ERROR::TEXTURE::LOAD_FAILED::"
            << texname
            << std::endl;
    }

    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    if (channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}
