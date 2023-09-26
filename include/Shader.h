#pragma once
#ifndef SHADER_H 
#define SHADER_H

#include "GL/glew.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
	unsigned int programID;	// The program ID

	// Constructor reads and builds the shader
	Shader(const char* vertexPath, const char* fragmentPath) {
		// 1.) Retrieve the vertex / fragment source code from filePath
		std::string vertexCode;		// Vertex code
		std::string fragmentCode;	// Fragment code

		std::ifstream vShaderFile;	// Vertex shader file
		std::ifstream fShaderFile;	// Fragment shader file

		// Ensure ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		// Try to open files
		try {
			// Open files
			vShaderFile.open(vertexPath);	// Open vertex shader file
			fShaderFile.open(fragmentPath); // Open fragment shader file
			std::stringstream vShaderStream, fShaderStream;  // String stream
			// Read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// Close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// Convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();

		// If there's an error, catch
		} catch (std::ifstream::failure e) {
			std::cout << "ERROR: SHADER FILE NOT SUCCESSFULLY READ" << std::endl;
		}

		const char* vShaderCode = vertexCode.c_str();   // Turn shader code into char*
		const char* fShaderCode = fragmentCode.c_str(); // Turn shader code into char*

		// 2.) Compile Shaders
		// Vertex shader
		unsigned int vertex;							// Vertex reference ID
		vertex = glCreateShader(GL_VERTEX_SHADER);		// Create shader
		glShaderSource(vertex, 1, &vShaderCode, NULL);	// Set shader source code to shader
		glCompileShader(vertex);						// Compile shader
		checkCompileErrors(vertex, "Vertex");			// Check for compilation errors

		// Fragment shader
		unsigned int fragment;							// Fragment reference ID
		fragment = glCreateShader(GL_FRAGMENT_SHADER);  // Create shader
		glShaderSource(fragment, 1, &fShaderCode, NULL);// Set shader source code to shader
		glCompileShader(fragment);						// Compile shader
		checkCompileErrors(fragment, "Fragment");		// Check for compilation errors

		// Shader Program
		programID = glCreateProgram();		// Create shader program
		glAttachShader(programID, vertex);	// Attach vertex shader
		glAttachShader(programID, fragment);// Attach fragment shader
		glLinkProgram(programID);			// Link program
		checkCompileErrors(programID, "Program");	// Check for linking errors

		// Delete the shaders as they're linked into our program
		glDeleteShader(vertex);		// Delete vertex shader
		glDeleteShader(fragment);	// Delete fragment shader
	};


	// Use / activate the shader
	void use() {
		glUseProgram(programID);
	};


	// Delete shader program
	void deleteProgram() {
		glDeleteProgram(programID); // Delete shader program
	};


	// Utility uniform functions
	// Set Boolean
	void setBool(const std::string& name, bool value) const {
		glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
	};

	void setInt(const std::string& name, int value) const {
		glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
	};

	void setFloat(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
	};

	void setMat4(const std::string& name, glm::mat4 transf) {
		unsigned int transformLoc = glGetUniformLocation(programID, name.c_str());
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transf));
	};


private: 
	// Check for compile errors
	void checkCompileErrors(GLuint shader, std::string type) {
		int success;		// Was it successful? 0 = no. 1 = yes.
		char infoLog[512];  // Infolog with error information
		// Shader compilation errors
		if (type != "Program") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 512, NULL, infoLog);
				std::cout << "ERROR: " << type << " shader compilation failed: " << infoLog << std::endl;
			}
		// Program linking errors
		} else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 512, NULL, infoLog);
				std::cout << "ERROR: Shader " << type << " linking failed: " << infoLog << std::endl;
			}
		}
	}
};

#endif
