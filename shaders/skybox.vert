#version 330 core
layout(location = 0) in vec3 vertPos;

out vec3 fragTex;

uniform mat4 projection;
uniform mat4 view;

void main() {
	// Removes translation component of view matrix
	vec4 pos =  projection * mat4(mat3(view)) * vec4(vertPos, 1.0);
	
	// Make skybox 'infinitely large' by tricking depth test
	gl_Position = pos.xyww;

	fragTex = vertPos;
}