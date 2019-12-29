#version 410 core

in vec3 normal;

out vec4 fColor;

void main() 
{	
	vec3 color = vec3(0.0, 0.5, 0.0);
    fColor = vec4(color, 1.0f);
}
