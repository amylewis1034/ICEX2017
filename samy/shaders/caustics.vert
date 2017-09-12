#version 330 core

layout (location = 0) in vec3 position;
layout (location = 3) in vec3 model_x;
layout (location = 4) in vec3 model_y;
layout (location = 5) in vec3 model_z;
layout (location = 6) in uvec3 indices;

uniform mat4 projection;
uniform mat4 view;

out vec4 fragPos;
flat out uvec3 fragIndices;

void main() {
    mat4 model = mat4(mat3(model_x, model_y, model_z));
    gl_Position = projection * view * model * vec4(position, 1);
    fragPos = gl_Position;
    fragIndices = indices;
}