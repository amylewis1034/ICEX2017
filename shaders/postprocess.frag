#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

uniform vec3 eye;

uniform sampler2D color_in;
uniform sampler2D depth;
uniform sampler2D bloom;

uniform float time;
uniform float gamma;
uniform float exposure;
uniform vec3 fog_color, be, bi;

uniform mat4 lastVP;
uniform sampler2D worldPos;
uniform float blurScale;

uniform float near, far;

out vec4 color;

//	<https://www.shadertoy.com/view/4dS3Wd>
//	By Morgan McGuire @morgan3d, http://graphicscodex.com
//
float hash(float n) { return fract(sin(n) * 1e4); }
float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(float x) {
	float i = floor(x);
	float f = fract(x);
	float u = f * f * (3.0 - 2.0 * f);
	return mix(hash(i), hash(i + 1.0), u);
}

vec3 gammacorrect(vec3 color) {
    if (gamma > 0)
        return pow(color, vec3(1.0 / gamma));
    else
        return color;
}

vec3 tonemap(vec3 color) {
	// return color / (color + vec3(1.0));

    if (exposure > 0.0) 
        return vec3(1.0) - exp(-color * exposure);
    else
        return color;
}

void main() {
    color = texture(color_in, fragTexcoord);
    // variable scrolling offset + contraction * fragTexcoord.x
    float noise = noise(fragTexcoord.x * 200 * (0.05* sin(time) + 0.5) + 5 * time - 10 * noise(time / 2));
    color.rgb += noise * vec3(0.3, 0.3, 0.5);
    return;

    float curDepth = texture(depth, fragTexcoord).r;
    // Linearize (http://glampert.com/2014/01-26/visualizing-the-depth-buffer/)
    curDepth = 2 * near / (far + near - curDepth * (far - near));
    vec3 worldPos = texture(worldPos, fragTexcoord).xyz;

    // Slightly modified from John Chapman's (http://john-chapman-graphics.blogspot.com/2013/01/what-is-motion-blur-motion-pictures-are.html)
    if (curDepth < .9) {
        vec4 lastPos = lastVP * vec4(worldPos, 1);
        lastPos.xyz /= lastPos.w;
        lastPos.xy = lastPos.xy * 0.5 + 0.5;

        vec2 blurVec = lastPos.xy - fragTexcoord;
        blurVec *= blurScale;

        vec4 result = texture(color_in, fragTexcoord);
        const int samples = 8;
        for (int i = 1;  i < samples; i++) {
            vec2 offset = blurVec * (float(i) / float(samples - 1) - 0.5);
            result += texture(color_in, fragTexcoord + offset);
        }
        result /= float(samples);
        color = result;
    }

    /* Fog */
    float linearDepth = curDepth;
    float fog = 0.0;

    /* linear fog */
    // float fogEnd = 0.9;      
    // float fogStart = 0.4;    
    // fog = 1.0 - clamp((fogEnd - linearDepth)/(fogEnd - fogEnd), 0.0, 1.0);

    /* exponential fog */
    // float fogDensity = 3;
    // fog = 1.0 - clamp(exp(-fogDensity*linearDepth), 0.0, 1.0); 
    
    // color.rgb = mix(color.rgb, vec3(0.7f), fog);

    /* Realistic Fog (http://www.iquilezles.org/www/articles/fog/fog.htm) */
    float dist = length(eye - worldPos);
// vec3 extColor = vec3( exp(-distance*be.x), exp(-distance*be.y) exp(-distance*be.z) );
// vec3 insColor = vec3( exp(-distance*bi.x), exp(-distance*bi.y) exp(-distance*bi.z) );
// finalColor = pixelColor*(1.0-extColor) + fogColor*insColor;
    // float be = (ten - worldPos.y) * factor1;
    // float bi = (ten - worldPos.y) * factor2;
    // float be = 0.25 * smoothstep(0.0, 6.0, 32.0 - worldPos.y);
    // float bi = 0.75 * smoothstep(0.0, 80, 10.0 - worldPos.y);

    vec3 extinction = vec3(exp(-dist * be.r), exp(-dist * be.g), exp(-dist * be.b));
    vec3 inscattering = vec3(exp(-dist * bi.r), exp(-dist * bi.g), exp(-dist * bi.b));
    // color.rgb = color.rgb * (1.0 - extinction) + fog_color * inscattering; 

    // color.rgb = color.rgb - vec3(0.6, 0.2, 0.1) * dist * .05;

    color.rgb = color.rgb * max(vec3((30 - dist) / 30, (80 - dist) / 80, (90 - dist) / 90), vec3(0));

    color = vec4(color.rgb + texture(bloom, fragTexcoord).rgb, 1);
    color = vec4(gammacorrect(tonemap(color.rgb)), 1.0);
    //color = vec4(texture(bloom, fragTexcoord).rgb, 1);
}
