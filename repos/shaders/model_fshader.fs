#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 DiffColor;  // Diffuse color

uniform sampler2D texture_diffuse1;

void main() {
	// Set fragment color to texture color and diffuse color
	FragColor = vec4(DiffColor, 1.0);
}