#pragma once
#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"

#include "Mesh.h"
#include "Shader.h"

#include <algorithm>    // std::max
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

class Model {
public:
	// Constructor
	Model(string path, vector<glm::vec3> sensor_pos_p) {
		sensor_pos = sensor_pos_p;	// Set sensor position vector
		loadModel(path);			// Load model
	};

	// Draw Meshes
	void Draw(Shader& shader, vector<float> data, int update_bool) {
		for (unsigned int i = 0; i < meshes.size(); i++) {
			meshes[i].Draw(shader, data, update_bool);
		}
	};

	// Clear Model
	void clearModel() {
		// Clear Meshes
		for (unsigned int i = 0; i < meshes.size(); i++) {
			meshes[i].clearMesh();
		}
		// Print meshes contents
		/*
		for (unsigned int i = 0; i < meshes.size(); i++) {
			printf("i: %d\n", i);
			printf("vertices: %lu\n", meshes[i].vertices.size());
			printf("indices: %lu\n", meshes[i].indices.size());
			printf("textures: %lu\n", meshes[i].textures.size());
		}*/
		//printf("meshes: %lu\n", meshes.size());
		meshes.clear();
		//printf("meshes: %lu\n", meshes.size());

		// Clear Textures
		/*
		printf("textures loaded: %lu\n", textures_loaded.size());
		for (unsigned int i = 0; i < textures_loaded.size(); i++) {
			printf("i: %d\n", i);
			printf("textures_loaded id: %d\n", textures_loaded[i].id);
		}*/
		textures_loaded.clear();
		//printf("textures loaded: %lu\n", textures_loaded.size());
	};


private:
	// Model Data
	vector<Mesh> meshes;	// Meshes
	string directory;		// Directory
	vector<Texture> textures_loaded;  // Textures we've already loaded
	vector<glm::vec3> sensor_pos;		// Sensor position

	// Load Model
	void loadModel(string path) {
		Assimp::Importer import;	// Load Assimp importer
		// Import scene. (Triangulate makes sure mesh is triangles. FlipUVs makes the textures flipped correctly)
		const aiScene* scene = import.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);

		// Check if import had issues
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			printf("ERROR: ASSIMP: %s\n", import.GetErrorString());
			return;
		}

		// If successful, then save directory and process nodes
		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, scene);	// Start by processing root node
	};


	// Process Node
	void processNode(aiNode* node, const aiScene* scene) {
		// Process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];  // Get mesh
			meshes.push_back(processMesh(mesh, scene));		 // Push processed mesh into mesh vector
		}
		// Then, do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);		// Process children nodes
		}
	};


	// Process interpolation data for a vertex given sensor_pos and vertex pos
	glm::vec4 calcVertexInterp(glm::vec3 vertex_pos) {
		// Initialize Data
		glm::vec4 interp_data = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);  // Initialize interp_data
		float west_num = 8;  // Number of sensor positions on the west side
		float roof_num = 8;  // Number of sensor positions on the roof
		float east_num = 9;  // Number of sensor poisitons on the east side
		float num = 0;		// Total number of sensors on the side

		// Go through each sensor positions. (in decreasing x order)
		for (int i = 0; i < (int)sensor_pos.size(); i++) {
			glm::vec3 temp_pos = sensor_pos.at(i);  // Get temp sensor pos

			// If vertex pos y is less than -1, then it's on the West side
			if (vertex_pos.y < -1) {
				num = west_num;
				if (vertex_pos.x > temp_pos.x) {	// When vertex x pos > a sensor x pos, 
					interp_data.x = i - 1;			// Set larger index to i - 1
					interp_data.y = i;				// Set smaller index to i
					break;
				}

			// If vertex pos y is between -1 and 1, then it's on the roof
			} else if (vertex_pos.y > -1 && vertex_pos.y < 1) {
				num = west_num + roof_num;
				if (vertex_pos.x > temp_pos.x) {	// When vertex x pos > a sensor x pos, 
					interp_data.x = i - 1;			// Set larger index to i - 1
					interp_data.y = i;				// Set smaller index to i
					break;
				}

			// Else, it's > 1 and on the East side
			} else {
				num = west_num + roof_num + east_num;
				if (vertex_pos.x > temp_pos.x) {	// When vertex x pos > a sensor x pos, 
					interp_data.x = i - 1;			// Set larger index to i - 1
					interp_data.y = i;				// Set smaller index to i
					break;
				}
			}//End if statement

			// If nothing was found, then it's between -9.4 and -11
			if (interp_data.x == -1 && interp_data.y == -1) {
				interp_data.x = num - 1.0f;		// Find index of largest one, which is in this case is 
			}

		}//End for loop

		// Find x points for interpolation
		float point_x1;
		if (interp_data.x == -1) {
			point_x1 = 12.0f;
		} else {
			point_x1 = sensor_pos.at((int)interp_data.x).x;  // x pos of sensor 1
		}
		float point_x2;
		if (interp_data.y == -1) {
			point_x2 = -12.0f;
		}
		else {
			point_x2 = sensor_pos.at((int)interp_data.y).x;  // x pos of sensor 2
		}

		// Find interpolations
		//printf("interp_data x: %f, %f, %f, %f\n", interp_data.x, interp_data.y, interp_data.z, interp_data.w);
		//printf("vertex pos x: %f, x1: %f, x2: %f\n", vertex_pos.x, point_x1, point_x2);
		interp_data.z = (point_x1 - vertex_pos.x) / (point_x1 - point_x2);	// Calculate blending for sensor 1
		interp_data.w = (vertex_pos.x - point_x2) / (point_x1 - point_x2);	// Calculate blending for sensor 2

		//printf("interp_data x: %f, %f, %f, %f\n", interp_data.x, interp_data.y, interp_data.z, interp_data.w);

		return interp_data;
	};


	// Process Mesh
	Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		vector<Vertex> vertices;  // Vertices for mesh
		vector<unsigned int> indices;  // Element indices for mesh
		vector<Texture> textures;	// Textures for mesh

		glm::vec4 diffuse_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);  // Diffuse color

		// Process materials
		// Check if material exists and if so, then process and add it to textures vector
		if (mesh->mMaterialIndex >= 0) {
			// Get material
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// Get diffuse color
			aiColor4D diffuse;  // Diffuse color in assimp form
			if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
				diffuse_color = glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);  // Extract color
			}

			// Load diffuse maps and add to textures vector
			vector<Texture> diffuseMaps = loadMaterialTextures(material,
				aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			// Load diffuse maps and add to textures vector
			vector<Texture> specularMaps = loadMaterialTextures(material,
				aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

			//diffuseMaps.clear();
			//specularMaps.clear();
		}

		// Loop through vertices and process them
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;  // New vertex
			// Process positions (They are put into a new vec3 to make sure the type is not weird)
			glm::vec3 position;
			position.x = mesh->mVertices[i].x;
			position.y = mesh->mVertices[i].y;
			position.z = mesh->mVertices[i].z;
			vertex.Position = position;
			vertex.originalPosition = position;
			// Process normals
			glm::vec3 normal;
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;
			vertex.Normal = normal;
			// Process texture coordinates (First see if the mesh even has them)
			if (mesh->mTextureCoords[0]) {
				glm::vec2 texCoords;
				texCoords.x = mesh->mTextureCoords[0][i].x;
				texCoords.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = texCoords;
			} else {
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}
			// Process Diffuse Color
			vertex.DiffuseColor = diffuse_color;

			// Process interpolation data
			vertex.interp_data = calcVertexInterp(vertex.Position);
			//printf("interp_data x: %f, %f, %f, %f\n", vertex.interp_data.x, vertex.interp_data.y, vertex.interp_data.z, vertex.interp_data.w);

			// Push vertex into vector of vertices for mesh
			vertices.push_back(vertex);
		}

		// Loop through each face and process indices (should always be a triangle)
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];  // Get face
			// Loop through each indice and add to indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		// Return Mesh
		return Mesh(vertices, indices, textures);
	};


	// Load Material Texture
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
		vector<Texture> textures;  // Textures vector that we will return at the end

		// Loop through, parse, and add each texture to the vector
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;						// Path to texture
			mat->GetTexture(type, i, &str);		// Get texture
			bool skip = false;					// Do we already have the texture and can skip?
			
			// Loop through loaded textures to see if there's a match
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				// If there's a match
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
					textures.push_back(textures_loaded[j]);  // Then push already loaded texture
					skip = true;	// Set skip to true, so it skips creating a new one
					break;
				}
			}

			// If the texture isn't already loaded, then we need to load it
			if (!skip) {
				Texture texture;					// Create new texture to push onto vector
				texture.id = TextureFromFile(str.C_Str(), directory);  // Load texture from directory
				texture.type = typeName;		// Texture type name
				texture.path = str.C_Str();				// Texture file path
				textures.push_back(texture);	// Push texture onto textures vector
				textures_loaded.push_back(texture);  // Push texture into already loaded vector
			}
		}

		return textures;  // Return textures vector
	};


	// Load texture from file
	unsigned int TextureFromFile(const char* path, string directory) {
		// Parse texture file name
		string filename = (string)path;			// Get filename
		filename = directory + '/' + filename;  // Get full file path

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
			} else if (nrChannels == 3) {
				format = GL_RGB;
			} else if (nrChannels == 4) {
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
		} else {
			printf("Texture failed to load at path: %s\n", filename.c_str());
		}

		stbi_image_free(data);  // Free image data

		return textureID;		// Return texture ID
	};


	// Clear private vectors
	//void clearPrivVecs() {
	//
	//}

};

#endif