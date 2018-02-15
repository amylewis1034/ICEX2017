#version 330 core

layout (location = 0) in vec3 position;
layout (location = 3) in vec4 model_x;
layout (location = 4) in vec4 model_y;
layout (location = 5) in vec4 model_z;
layout (location = 6) in vec4 model_trans;
layout (location = 7) in uvec3 indices;

uniform mat4 projection;
uniform mat4 view;

out vec4 fragPos;
out vec4 fragWorldPos;
flat out ivec3 fragIndices;

void main() {
    mat4 model = mat4(model_x, model_y, model_z, model_trans);
    gl_Position = projection * view * model * vec4(position, 1);
    fragPos = gl_Position;
    fragWorldPos = model * vec4(position, 1);
    fragIndices = ivec3(indices);
}
