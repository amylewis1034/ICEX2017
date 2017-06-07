#version 330 core 
in vec3 fragNorRaw;

layout(location = 0) out vec4 color;


void main()
{
    // Re-Normalize
    vec3 fragNor = normalize(fragNorRaw);

    // Map normal in the range [-1, 1] to color in range [0, 1];
    color = vec4(fragNor, 1.0);
}
