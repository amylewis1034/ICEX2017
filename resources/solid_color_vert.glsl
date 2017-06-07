#version  330 core
layout(location = 0) in vec4 vertPos;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main()
{
    gl_Position = P * V * M * vertPos;
    gl_PointSize = 10.0;
}