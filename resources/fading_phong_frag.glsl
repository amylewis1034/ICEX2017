#version 330 core 
uniform vec3 lightCol1;
uniform vec3 lightCol2;
uniform vec3 matAmb;
uniform vec3 matDif;
uniform vec3 matSpec;
uniform float matShine;
uniform float viewDist;
uniform float baseAlpha;
uniform sampler2D water1;
uniform sampler2D water2;
uniform float interp;
in vec3 fragNorRaw;
in vec3 lightVecRaw;
in vec3 lightVecRaw2;
in vec3 halfVecRaw;
in vec3 halfVecRaw2;
in vec3 vertPosWorld;
in float dist;
layout(location = 0) out vec4 color;

in vec4 caust_pos;

void main()
{
	vec4 caustTex = caust_pos / caust_pos.w;
	caustTex *= 0.5;
	caustTex += 0.5;
	vec3 caustColor = texture(water1, caustTex.xy).rgb * (1 - interp) + texture(water2, caustTex.xy).rgb * interp;
    
	if (vertPosWorld.y < 0.0f) {
	   discard;
	}

	vec3 fragNor = normalize(fragNorRaw);
	vec3 lightVec = normalize(lightVecRaw);
	vec3 halfVec = normalize(halfVecRaw);
    
	vec3 lightVec2 = normalize(lightVecRaw2);
	vec3 halfVec2 = normalize(halfVecRaw2);

	// Compute Phong color.
	vec3 diffuseCol = lightCol1 * max(dot(fragNor, lightVec), 0.0f) * matDif;
	vec3 specularCol = lightCol1 * pow(max(dot(fragNor, halfVec), 0.0f), matShine) * matSpec;
	diffuseCol += lightCol2 * max(dot(fragNor, lightVec2), 0.0f) * matDif;
	specularCol += lightCol2 * pow(max(dot(fragNor, halfVec2), 0.0f), matShine) * matSpec;
	vec3 vertCol = diffuseCol + specularCol + matAmb;

	// Compute Fade alpha.
	float alpha = 1.0f;
	float fadeBegin = 0.5f * viewDist;
	float fadeEnd = viewDist;
	if (dist > fadeBegin) {
	   alpha = 1.0f - (dist - fadeBegin)/(fadeEnd - fadeBegin);
	}
    
	vec3 redGreen = vec3(0.4f, 0.3f, 0.3f);
	vec3 finalColor = vertCol - 0.5 * (redGreen * log(dist));
	finalColor += vec3(0.0f, 0.0f, 0.1f);

	color = vec4(finalColor + (caustColor * 0.8), max((alpha * baseAlpha) - 0.2, 0.0f));
}
