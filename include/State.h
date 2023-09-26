#pragma once
#ifndef GUI_H
#define GUI_H

#include <GL/glew.h>  // Holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

#include "Shader.h"

#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
using namespace std;


// Texture
struct GuiTexture {
	unsigned int id;		// Texture id
	string path;			// Path to the texture to compare with other textures
	glm::vec2 position;		// Texture position
	glm::vec2 scale;		// Texture scale
};

// Button
struct Button {
	int state_num;  // State number button goes to
	glm::vec4 pos;  // Position of button. pos = x, y, width, height
};


class State {
public:
	// State Constructor
	State(unsigned int state_num, string state_txt) {
		State_num = state_num;
		state_txtname = state_txt;
		setupQuadBuffer();	// Set up Quad buffer for textures

	}


	// Load Material Texture
	void loadMaterialTextures(string filepath, glm::vec2 pos, glm::vec2 scale) {

		bool skip = false;	// Do we already have the texture and can skip?

		// Loop through loaded textures to see if there's a match
		for (unsigned int i = 0; i < textures_loaded.size(); i++) {
			// If there's a match
			if (textures_loaded[i].path == filepath) {
				GuiTexture texture = textures_loaded[i];
				texture.position = pos;		// Update position
				texture.scale = scale;		// Update scale

				textures.push_back(texture);  // Then push already loaded texture
				skip = true;	// Set skip to true, so it skips creating a new one
				break;
			}
		}

		// If the texture isn't already loaded, then we need to load it
		if (!skip) {
			GuiTexture texture;					// Create new texture to push onto vector
			texture.id = TextureFromFile(filepath);  // Load texture from directory
			texture.path = filepath;				// Texture file path
			texture.position = pos;					// Texture position
			texture.scale = scale;					// Texture scale
			textures.push_back(texture);	// Push texture onto textures vector
			textures_loaded.push_back(texture);  // Push texture into already loaded vector
		}

	};



	// Add button to state
	void addButton(int astate_num, glm::vec4 apos) {
		Button temp_button;						// Create a button to add to the array
		temp_button.state_num = astate_num;
		temp_button.pos = apos;
		buttons.push_back(temp_button);  // Add button to the button array
	};



	// Figure out if button is clicked and return state number (or -2 if it doesn't return anything)
	int handle_events(SDL_Event& event) {

		int mouse_x = 0;  // Mouse offsets
		int mouse_y = 0; 

		// If a mouse button was pressed and it was the left one
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT) {

				// Get mouse offsets
				mouse_x = event.button.x;
				mouse_y = event.button.y;

				for (int i = 0; i < buttons.size(); i++) {

					Button curr = buttons[i];  // Extract button

					// If the mouse is over the button
					if ((mouse_x > curr.pos[0]) && (mouse_x < curr.pos[0] + curr.pos[2]) && (mouse_y > curr.pos[1]) && (mouse_y < curr.pos[1] + curr.pos[3])) {
						return curr.state_num;
					}
				}
			}
		}
		return -2;
	};



	// Draw
	void draw(Shader& shader) {

		glDisable(GL_DEPTH_TEST);  // Disable depth testing with z buffers

		// Set texture uniform in shader
		shader.setInt("texture0", 0); // Set shader float uniform for texture

		// Activate proper texture unit before binding
		glBindVertexArray(VAO);		// Bind vertex attrib array

		glActiveTexture(GL_TEXTURE0);

		// Loop through each texture
		for (unsigned int i = 0; i < textures.size(); i++) {

			// Set transformation matrix
			glm::mat4 transf = glm::mat4(1.0f); // Start as identity matrix
			transf = glm::translate(transf, glm::vec3(textures[i].position, 0.0f));  // Translate
			transf = glm::scale(transf, glm::vec3(textures[i].scale, 1.0));  // Scale
			shader.setMat4("transformation", transf);	// Set uniform in shader

			// Bind texture and vertex array
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);

			// Draw quad
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  // Draw vertices
			
		}//End for loop

		// Unbind
		glBindVertexArray(0);		// Unbind vertex attrib array

		glEnable(GL_DEPTH_TEST);  // Enable depth testing with z buffers

	};



private:
	vector<GuiTexture> textures;			// Textures
	vector<GuiTexture> textures_loaded;  // Already loaded textures
	unsigned int State_num;				// State id number
	string state_txtname;					// State text file where texture information is
	unsigned int VAO, VBO;				// Vertex array buffer and vertex buffer
	vector<Button> buttons;				// Array of buttons


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

		//printf("state vao: %d\n", VAO);

		// Bind arrays and buffers and populate them with data
		glBindVertexArray(VAO);		// Bind vertex attrib array

		glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Bind vertex buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  // Buffer data

		// Vertex Positions
		glEnableVertexAttribArray(0);  // Enable vertex positions attribute
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);  // Set up attribute pointer

		glBindVertexArray(0);  // Unbind vertex attrib array
	};


	// Load State by Reading Text File
	void loadState() {
		
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