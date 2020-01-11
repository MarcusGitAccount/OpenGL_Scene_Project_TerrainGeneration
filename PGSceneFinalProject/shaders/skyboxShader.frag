#version 410 core

in vec3 textureCoordinates;
out vec4 color;
in vec4 fragmentPosEyeSpace; // view * model * vertexPosition
uniform samplerCube skybox;

float computeFog()
{
 float fogDensity = 1.55f;
 float fragmentDistance = length(fragmentPosEyeSpace);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

void main()
{
	float scale = 0.5f;
    vec4 temp = texture(skybox, textureCoordinates);
	float factor = computeFog() * scale;
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	color = fogColor * (1 - factor) + temp * factor; // interpolare
}
