#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;

out vec3 fragPos;
out vec4 fragColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(pos, 1.0);
    fragPos = (projection * view * model * vec4(pos, 1.0)).xyz;
    fragColor = color;
}
