#version 330 core 
//uniform float viewDist;
//uniform vec3 lightCol;
//uniform float lightCol[6];
uniform vec3 lightCol1;
uniform vec3 lightCol2;
uniform float matAmb;
uniform float matDif;
uniform vec3 matSpec;
uniform float matShine;
uniform bool isAgisoftModel;
uniform float interp;
//uniform float brightness; //with 1 being full and 0 being completely dark
uniform sampler2D texture0;
uniform sampler2D water1;
uniform sampler2D water2;
in vec3 fragNorRaw;
in vec2 fragTex;
in vec3 lightVecRaw1;
in vec3 lightVecRaw2;
in vec3 halfVecRaw1;
in vec3 halfVecRaw2;
in vec3 lightVecRaw[2];
in vec3 halfVecRaw[2];
in float dist;
in float light2Dist;
in vec3 vertPosWorld;
// out vec4 color;
layout(location = 0) out vec4 color;

in vec4 caust_pos;
uniform bool caust;

void main()
{
    vec4 caustTex = caust_pos / caust_pos.w;
    caustTex *= 0.5;
    caustTex += 0.5;
    vec3 caustColor = texture(water1, caustTex.xy).rgb * (1 - interp) + texture(water2, caustTex.xy).rgb * interp;

    if (vertPosWorld.y < 0.0f) {
       discard;
    }

    // Re-Normalize
    vec3 fragNor = normalize(fragNorRaw);
    vec3 lightVec[2];
    vec3 lightVec1 = normalize(lightVecRaw1);
    vec3 lightVec2 = normalize(lightVecRaw2);
    vec3 halfVec1 = normalize(halfVecRaw1);
    vec3 halfVec2 = normalize(halfVecRaw2);

    //lightVec[0] = normalize(lightVecRaw[0]);
    //lightVec[1] = normalize(lightVecRaw[1]);
    //vec3 halfVec[2];
    //halfVec[0] = normalize(halfVecRaw[0]);
    //halfVec[1] = normalize(halfVecRaw[1]);

    // Compute Phong color.
    // fragNor = normalize(fragNor);

    // vec3 tex = texture(texture0, fragTex).rgb;
    // vec3 diffuseCol = lightCol * max(dot(fragNor, lightVec), 0.0f) * matDif * tex;
    // vec3 specularCol = lightCol * pow(max(dot(fragNor, halfVec), 0.0f), matShine) * matSpec;
    // vec3 ambientCol = matAmb * tex;
    // vec3 vertCol = diffuseCol + specularCol + ambientCol;

    fragNor = normalize(fragNor);

    vec3 tex = texture(texture0, fragTex).rgb;
    vec3 ambientCol = matAmb * tex;
    vec3 diffuseCol;
    vec3 specularCol;
    vec3 vertCol = ambientCol * 3.0f;
    diffuseCol = lightCol1 * max(dot(fragNor, lightVec1), 0.0f) * matDif * tex;
    diffuseCol += lightCol2 * 30.0f/(light2Dist) * max(dot(fragNor, lightVec2), 0.0f) * matDif * tex;

    specularCol = lightCol1 * pow(max(dot(fragNor, halfVec1), 0.0f), matShine) * matSpec;
    specularCol += lightCol2 * 30.0f/(light2Dist) * pow(max(dot(fragNor, halfVec2), 0.0f), matShine) * matSpec;

    vertCol += diffuseCol + specularCol;

    vec3 redGreen = vec3(0.6f, 0.2f, 0.1f);
	vec3 finalColor = vertCol - (redGreen * 0.2f*dist);
    caustColor -= 0.3*(redGreen * log(dist));
    finalColor += vec3(0.0f, 0.0f, 0.1f);

    vec3 seaFloorColor = vec3(0.33f, 0.34f, 0.64f);
    if (isAgisoftModel){ // && dot(fragNor, vec3(0, 1, 0)) < 0.3){
        finalColor -= 0.2f * ((vec3(1.0f, 1.0f, 1.0f) - seaFloorColor) * max(6 - vertPosWorld.y, 0.0f));
    }

    if (caust) {
        color = vec4(finalColor + (caustColor * 0.8), 1.0f);
    } else {
        color = vec4(finalColor, 1.0f);
    }
}
