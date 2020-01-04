#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 normal;
out vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	if (vPosition.y < 0.f) {
		color = vec3(0.f, 0.f, 1.f);
	} else if (vPosition.y < 0.4f) {
		color = vec3(0.f, 0.5f, 0.5f);
	} else if (vPosition.y < 0.6f) {
		color = vec3(0.f, 1.f, 0.f);
	} else {
		color = vec3(1.f, 1.f, 1.f);
	}

	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
}
