#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

uniform sampler2D texture0;
uniform float alphaScale = 1.0;

out vec4 color;

void main() {
    color = texture(texture0, fragTexcoord);
    color.a *= alphaScale;
}
