#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "Component.hpp"
#include <Graphics/GLTexture.hpp>

#include <string>

class Texture : public Component {
public:
	// Texture();
	Texture(std::string texname);
	~Texture();

	void init();
	void update(float dt);

	void bind();
	void unbind();

	
private:
	GLTexture texture;
	std::string texname;
};

#endif
