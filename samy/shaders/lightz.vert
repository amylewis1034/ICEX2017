#version 330 core

layout (location = 0) in vec4 position;

uniform mat4 projection;
uniform mat4 view;

out vec3 geomPos;

void main() {
    geomPos = (view * position).xyz;
    gl_Position = projection * view * position;
}
