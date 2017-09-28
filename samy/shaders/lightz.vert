#version 330 core

layout (location = 0) in vec4 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 lv;

void main() {
    // gl_Position = projection * view * inverse(lv) * position;
    gl_Position = position;
}
