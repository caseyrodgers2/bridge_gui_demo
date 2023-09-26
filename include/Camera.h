#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as an
// abstration to stay away from window-system specific input methods
enum Camera_Movement {
	// Position
	MOVE_FORWARD,
	MOVE_BACKWARD,
	MOVE_LEFT,
	MOVE_RIGHT,

	// Euler Angle
	TURN_LEFT,
	TURN_RIGHT,
	TURN_UP,
	TURN_DOWN,

	// Zoom
	ZOOM_IN,
	ZOOM_OUT
};

// Default Camera Values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 20.0f;
const float SENSITIVITY = 1.0f;
const float FOV = 45.0f;		// Aka zoom


// An abstract camera class that processes input and calculates the
// corresponding Euler Angles, Vectors, and Matrices for use in OpenGL
class Camera {
public:
	// Camera attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera Options
	float MovementSpeed;
	float Sensitivity;
	float Fov;

	// Constructor with Vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
		: Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), Sensitivity(SENSITIVITY), Fov(FOV) {
		// Set variables
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	};

	// Constructor with Scalar Values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
		: Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), Sensitivity(SENSITIVITY), Fov(FOV) {
		// Set variables
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	};

	// Returns the View Matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix() {
		return glm::lookAt(Position, Position + Front, Up);
	};

	// Process Input received from any keyboard-like input system.
	// Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime) {

		float velocity = MovementSpeed * deltaTime;  // For Position

		int positionChange = 0;  // Did the position change?
		int angleChange = 0;	// Did the angle change?
		int zoomChange = 0;		// Did the zoom change?

		switch (direction) {

			/// Turn Different Angles
			// Rotate camera to the right
		case TURN_RIGHT:
			Yaw += Sensitivity;
			angleChange = 1;
			break;
			// Rotate camera to the left
		case TURN_LEFT:
			Yaw -= Sensitivity;
			angleChange = 1;
			break;
			// Rotate camera upwards
		case TURN_UP:
			Pitch += Sensitivity;
			angleChange = 1;
			break;
			// Rotate camera downwards
		case TURN_DOWN:
			Pitch -= Sensitivity;
			angleChange = 1;
			break;

			/// Move Position
			// Move camera forward
		case MOVE_FORWARD:
			Position += Front * velocity;
			positionChange = 1;
			break;
			// Move camera backward
		case MOVE_BACKWARD:
			Position -= Front * velocity;
			positionChange = 1;
			break;
			// Move camera to the left
		case MOVE_LEFT:
			Position -= Right * velocity;
			positionChange = 1;
			break;
			// Move camera to the right
		case MOVE_RIGHT:
			Position += Right * velocity;
			positionChange = 1;
			break;

			/// Zoom
			// Zoom In
		case ZOOM_IN:
			Fov -= Sensitivity;
			zoomChange = 1;
			break;
			// Zoom Out
		case ZOOM_OUT:
			Fov += Sensitivity;
			zoomChange = 1;
			break;
		default:
			break;

		}//End switch

		// Angle Constraints
		if (angleChange == 1) {
			if (Pitch > 89.0f) {
				Pitch = 89.9f;
			}
			else if (Pitch < -89.0f) {
				Pitch = -89.0f;
			}
			updateCameraVectors(); // Update camera vectors
		}

		// Zoom Constraints
		if (zoomChange == 1) {
			if (Fov < 1.0f) {
				Fov = 1.0f;
			}
			else if (Fov > 45.0f) {
				Fov = 45.0f;
			}
		}

	};//End function


private:
	// Calculate the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors() {
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);  // Normalize

		// Re-calculate the Right and Up vector
		// Normalize the vectors bc their length gets closer to 0 the more you
		// look up or down which results in slower movement
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	};

};//End class

#endif