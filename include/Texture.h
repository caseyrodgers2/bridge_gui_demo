#pragma once
#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>  // Holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

#include "Shader.h"

#include <string>
#include <vector>
#include <stdio.h>
using namespace std;


class Texture {
public:
	// Texture Constructor
	Texture(string filepath) {
		setupQuadBuffer();	// Set up Quad buffer for textures
	}


	// Draw
	void draw(Shader& shader) {

		glDisable(GL_DEPTH_TEST);  // Disable depth testing with z buffers

		// Set texture uniform in shader
		shader.setInt("texture0", 0); // Set shader float uniform for texture

		for (int i = 0; i < positions.size(); i++) {

			// Set transformation matrix
			glm::mat4 transf = glm::mat4(1.0f); // Start as identity matrix
			transf = glm::translate(transf, glm::vec3(positions[i], 0.0f));  // Translate
			transf = glm::scale(transf, glm::vec3(scales[i], 1.0));  // Scale
			shader.setMat4("transformation", transf);	// Set uniform in shader

			// Activate proper texture unit before binding
			glBindVertexArray(VAO);		// Bind vertex attrib array
			glActiveTexture(GL_TEXTURE0);

			// Bind texture and vertex array
			glBindTexture(GL_TEXTURE_2D, id);

			// Draw quad
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  // Draw vertices

			glActiveTexture(GL_TEXTURE0);  // Reset active texture

			glBindVertexArray(0);		// Unbind vertex attrib array

		}

		// Unbind
		glActiveTexture(GL_TEXTURE0);  // Reset active texture

		glEnable(GL_DEPTH_TEST);	// Enable depth testing with z buffers

	};



private:
	unsigned int id;		// Texture id
	string path;			// Path to the texture to compare with other textures
	vector<glm::vec2> positions;		// Texture position
	vector<glm::vec2> scales;		// Texture scale

	unsigned int VAO, VBO;				// Vertex array buffer and vertex buffer


	// Set up Quad Buffer (since most things we're doing are for gui textures)
	void setupQuadBuffer() {
		// Vertices
		float vertices[] = {
			-1.0f,  1.0f,
			-1.0f, -1.0f,
			 1.0f,  1.0f,
			 1.0f, -1.0f
		};

		// Generate buffers and arrays
		glGenVertexArrays(1, &VAO);		// Generate vertex attrib arrays
		glGenBuffers(1, &VBO);			// Generate vertex buffer

		// Bind arrays and buffers and populate them with data
		glBindVertexArray(VAO);		// Bind vertex attrib array

		glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Bind vertex buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  // Buffer data

		// Vertex Positions
		glEnableVertexAttribArray(0);  // Enable vertex positions attribute
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);  // Set up attribute pointer

		glBindVertexArray(0);  // Unbind vertex attrib array
	};


	// Load Material Texture
	void loadMaterialTextures(string filepath, glm::vec2 pos, glm::vec2 scale) {

		id = TextureFromFile(filepath);  // Load texture from directory
		path = filepath;				// Texture file path
		positions.push_back(pos);		// Texture position
		scales.push_back(scale);		// Texture scale

	};


	// Load texture from file
	unsigned int TextureFromFile(string path) {
		// Parse texture file name
		string filename = path;  // Get full file path

		// Generate texture
		unsigned int textureID;
		glGenTextures(1, &textureID);

		// Load a texture
		int width, height, nrChannels;  // Texture width, height, and num channels
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);  // Load texture

		// Check if image data loaded okay
		// If so, then attach texture image to texture and create mipmap
		if (data) {
			// Figure out image format based on num channels
			GLenum format;
			if (nrChannels == 1) {
				format = GL_RED;
			}
			else if (nrChannels == 3) {
				format = GL_RGB;
			}
			else if (nrChannels == 4) {
				format = GL_RGBA;
			}

			glBindTexture(GL_TEXTURE_2D, textureID);  // Bind texture
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);  // Attach texture image to texture
			glGenerateMipmap(GL_TEXTURE_2D);	// Generate mipmap

			// Set texture wraping / filtering options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Texture wrapping for s coord
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  // Texture wrapping for t coord
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  // Texture filtering when downscaling
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // Texture filtering for upscaling

		// Otherwise, image did not load correctly, so print error
		}
		else {
			printf("Texture failed to load at path: %s\n", filename.c_str());
		}

		stbi_image_free(data);  // Free image data

		return textureID;		// Return texture ID
	};






};
#endif