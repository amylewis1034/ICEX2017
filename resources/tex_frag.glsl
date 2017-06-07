#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;

void main() {
   // Don't put in blue component to make it easy to tell this code is running
   // color = vec4(texture( texBuf, texCoord ).rg, 0, 1);
   color = vec4(texture( texBuf, texCoord ).rgb, 1);
}
