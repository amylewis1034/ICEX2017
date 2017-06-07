#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 fragNorRaw;

void main()
{
	vertTex; // To silence shader warning
	vec4 viewPos4 = V * M * vertPos;
	gl_Position = P * viewPos4;
	// Multiply normals by view and model matrix to make normals 
	// point towards camera
	fragNorRaw = (V * M * vec4(vertNor, 0.0)).xyz;
}
