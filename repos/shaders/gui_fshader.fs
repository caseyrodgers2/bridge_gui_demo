#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture0;

void main() {
	// Set fragment color to texture color and diffuse color
	FragColor = texture(texture0, TexCoord);
}