#pragma once
#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>  // Holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

// Vertex struct
struct Vertex {
	glm::vec3 Position;		// Position
	glm::vec3 Normal;		// Normal
	glm::vec2 TexCoords;	// Texture coordinates
	glm::vec3 DiffuseColor; // Diffuse colors
	glm::vec3 originalPosition;		// Original Position
	glm::vec4 interp_data;  // Data needed for interpolation. 0, 1 = indices of nearest sensors. 2, 3 = percent contribution from nearest sensors
};

// Texture
struct Texture {
	unsigned int id;	// Texture id
	std::string type;		// Texture type
	std::string path;	// Path to the texture to compare with other textures
};


// Mesh Class
class Mesh {
public:
	// Mesh Data
	std::vector<Vertex> vertices;		// Vertices vector
	std::vector<unsigned int> indices;	// Indices vector
	std::vector<Texture> textures;		// Textures vector

	// Mesh Constructor
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
		this->vertices = vertices;	// Set vertices
		this->indices = indices;	// Set indices
		this->textures = textures;	// Set textures
		setupMesh();				// Set up mesh
	};

	// Draw
	void Draw(Shader& shader, std::vector<float> data, int update_bool) {

		if (update_bool) {
			updateMesh(data);
		}
		//printf("after update mesh \n");

		unsigned int diffuseNr = 1;		// Diffuse texture number
		unsigned int specularNr = 1;	// Specular texture number

		// Bind vertex array
		glBindVertexArray(VAO);		// Bind vertex attrib array

		// Loop through each texture
		/*
		for (unsigned int i = 0; i < textures.size(); i++) {
			printf("here\n");
			glActiveTexture(GL_TEXTURE0 + i);	// Activate proper texture unit before binding

			// Put texture name together
			std::string number;		// Texture number (the N in diffuse_textureN)
			std::string name = textures[i].type;	// Texture type
			//printf("texture name: %s\n", name.c_str());
			// Diffuse texture
			if (name == "texture_diffuse") {
				number = std::to_string(diffuseNr++);  // Convert number to string, then increment number
			// Specular texture
			} else if (name == "texture_specular") {
				number = std::to_string(specularNr++); // Convert number to string, then increment
			}

			shader.setInt((name + number).c_str(), i); // Set shader float uniform for texture
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}//End for loop
		*/

		// Draw Mesh
		glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);  // Draw elements

		glBindVertexArray(0);		// Unbind vertex attrib array

		glActiveTexture(GL_TEXTURE0);  // Reset active texture
	};


	// Update Mesh vertices
	// data = vector of floats for sensor values
	void updateMesh(std::vector<float> data) {
		// Find max and min data values
		float max_value = 1;		// Max value
		float min_value = -1;	// Min value
		/*for (int i = 0; i < data.size(); i++) {
			if (data.at(i) > max_value) {
				max_value = data.at(i);
			}
			if (data.at(i) < min_value) {
				min_value = data.at(i);
			}
		}*/

		// Go through each vertex and update its position and color
		for (int i = 0; i < vertices.size(); i++) {
			Vertex temp_vertex = vertices.at(i);  // Get current vertex

			// Find new displacement
			float blending1 = temp_vertex.interp_data.z;	// Blending for sensor 1
			float blending2 = temp_vertex.interp_data.w;	// Blending for sesnor 2

			//for (int i = 0; i < data.size(); i++) {
			//	printf("data: %f\n", data.at(i));
			//}

			//printf("interp_data x: %f, %f, %f, %f\n", temp_vertex.interp_data.x, temp_vertex.interp_data.y, temp_vertex.interp_data.z, temp_vertex.interp_data.w);
			
			float data1;
			if (temp_vertex.interp_data.x == -1) {
				data1 = 0;
			}else {
				data1 = data.at(temp_vertex.interp_data.x);  // Data for closest sensor 1
			}
			
			float data2;
			if (temp_vertex.interp_data.y == -1) {
				data2 = 0;
			}
			else {
				data2 = data.at(temp_vertex.interp_data.y);  // Data for closest sensor 2
			}

			float delta_displ = blending1 * data1 + blending2 * data2;	// Change in displacement
			//printf("delta_displ: %f\n", delta_displ);
			//temp_vertex.Position.y = temp_vertex.originalPosition.y + delta_displ;		// Add in change in displacement

			// Find new color (max = red (0). min = blue(240/360))
			float hue = (1 - delta_displ / (max_value - min_value)) * 240;  // New hue
			float C = 1;		// C = V * S = 1 * 1
			float X = C * (1 - abs(((int)(hue / 60)) % 2 - 1.0f));
			float m = 1 - C;	// m = V - C = 1 - C

			glm::vec3 new_diff_color;
			if (hue < 60) {
				new_diff_color = glm::vec3(C, X, 0);
			} else if (hue < 120) {
				new_diff_color = glm::vec3(X, C, 0);
			} else if (hue < 180) {
				new_diff_color = glm::vec3(0, C, X);
			} else {
				new_diff_color = glm::vec3(0, X, C);
			}

			temp_vertex.DiffuseColor = new_diff_color;

			vertices.erase(vertices.begin() + i);
			vertices.insert(vertices.begin() + i, temp_vertex);
		}

		// Update vertex arrays
		glBindVertexArray(VAO);		// Bind vertex attrib array
		glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Bind vertex buffer
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);  // Buffer data
		glBindVertexArray(0);		// Unbind vertex attrib array
	};


	// Clear Mesh
	void clearMesh() {
		// Clear vectors
		vertices.clear();
		indices.clear();
		textures.clear();
	};



private:
	// Render data
	unsigned int VAO, VBO, EBO;  // Vertex attribute array, vertex buffer, element indices buffer

	// Set up Mesh
	void setupMesh() {
		// Generate buffers and arrays
		glGenVertexArrays(1, &VAO);		// Generate vertex attrib arrays
		glGenBuffers(1, &VBO);			// Generate vertex buffer
		glGenBuffers(1, &EBO);			// Generate element buffer

		//printf("mesh vao: %d\n", VAO);

		// Bind arrays and buffers and populate them with data
		glBindVertexArray(VAO);		// Bind vertex attrib array

		glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Bind vertex buffer
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);  // Buffer data

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);  // Bind element buffer
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
			&indices[0], GL_STATIC_DRAW);

		// Vertex Positions
		glEnableVertexAttribArray(0);  // Enable vertex positions attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));  // Set up attribute pointer

		// Vertex Normals
		glEnableVertexAttribArray(1);  // Enable vertex normals attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));  // Set up attribute pointer

		// Vertex Texture Coords
		glEnableVertexAttribArray(2);  // Enable vertex texture coords attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));  // Set up attribute pointer

		// Vertex Diffuse Color
		glEnableVertexAttribArray(3);  // Enable vertex texture coords attribute
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, DiffuseColor));  // Set up attribute pointer

		// Disable and unbind arrays
		glBindVertexArray(0);  // Unbind vertex attrib array
	};
};

#endif
