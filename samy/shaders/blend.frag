#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

out vec4 color;

uniform sampler2D color_in;
uniform bool horizontal;

// slightly modified from https://learnopengl.com/#!Advanced-Lighting/Bloom
// will be modifying with http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
// and http://kalogirou.net/2006/05/20/how-to-do-good-bloom-for-hdr-rendering/
// and possibly Real Time Rendering to get a better result
void main() {
    color = vec4(texture(color_in, fragTexcoord).rgb, 1);
    return;

    const float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.01626);
    vec2 offset = 1.0 / textureSize(color_in, 0);
    
    vec3 blurred = texture(color_in, fragTexcoord).rgb * weights[0];
    if (horizontal) {
        for (int i = 1; i < 5; i++) {
            blurred += texture(color_in, fragTexcoord + vec2(offset.x * i, 0)).rgb * weights[i];
            blurred += texture(color_in, fragTexcoord - vec2(offset.x * i, 0)).rgb * weights[i];
        }
    }
    else {
        for (int i = 1; i < 5; i++) {
            blurred += texture(color_in, fragTexcoord + vec2(0, offset.y * i)).rgb * weights[i];
            blurred += texture(color_in, fragTexcoord - vec2(0, offset.y * i)).rgb * weights[i];
        }
    }

    color = vec4(blurred, 1.0);
}
