#version 330 core
layout (location = 0) in vec2 aPos;  // The position variable has attribute position 0

out vec2 TexCoord;		// Texture coords

uniform mat4 transformation;

void main() {
	gl_Position = transformation * vec4(aPos, 0.0, 1.0);
	TexCoord = vec2((aPos.x + 1)/2, (aPos.y + 1)/2);
}