#version 330 core
layout (location = 0) in vec3 aPos;  // The position variable has attribute position 0
layout (location = 1) in vec3 aNormal;  // The normal has attribute position 1
layout (location = 2) in vec2 aTexCoord;	// Texture coords has attribute position 2
layout (location = 3) in vec3 aDiffColor;  // Diffuse color has attribute position 3

out vec2 TexCoord;	// Output texture coordinates to the fragment shader
out vec3 DiffColor; // Output diffuse color to the fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	TexCoord = aTexCoord; // Set TexCoord to the input tex coord from vertex data
	DiffColor = aDiffColor;  // Set DiffColor to the input diffuse color from vertex data
}