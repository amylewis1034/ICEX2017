#include "Texture.hpp"
#include <Graphics/GLTexture.hpp>

#include <string>

// Texture::Texture() : {}

Texture::Texture(std::string texname) :
	texname(texname) {}

Texture::~Texture() {}


void Texture::init() {
	// assert(texname);
	texture.setTarget(GL_TEXTURE0);
	texture.loadTexture(texname);
}

void Texture::update(float dt) {}

void Texture::bind() {
	texture.bind();
}

void Texture::unbind() {
	texture.unbind();
}