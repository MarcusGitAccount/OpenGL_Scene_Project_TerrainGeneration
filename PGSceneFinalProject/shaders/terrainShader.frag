#version 410 core

in vec3 normal;
in vec3 color;

out vec4 fColor;

void main() 
{	
    fColor = vec4(color, 1.0f);
}
