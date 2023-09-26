#include <SDL.h>
#include <SDL_mixer.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#include "include/Shader.h"
#include "include/Model.h"
#include "include/Camera.h"
#include "include/State.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// Process State Input
int processStateInput(SDL_Event event, State temp_state) {

	// If person hits key on keyboard
	if (event.type == SDL_KEYDOWN) {

		// If key is escape key, then exit game
		if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			return -1;
		}

	// Mouse movement
	} else {
		return temp_state.handle_events(event);
	}

	return -2;
}


// Process Input to Change Camera. (Updates and then returns Camera)
Camera processCamInput(float deltaTime, Camera camera) {

	const Uint8* keystates = SDL_GetKeyboardState(NULL);  // Get keys that are currently pressed down

	/// Move Position
	// If key is W, then move forward
	if (keystates[SDL_SCANCODE_W]) {
		camera.ProcessKeyboard(MOVE_FORWARD, deltaTime);
	}
	// If key is S, then move backward
	if (keystates[SDL_SCANCODE_S]) {
		camera.ProcessKeyboard(MOVE_BACKWARD, deltaTime);
	}
	// If key is A, then move to the left
	if (keystates[SDL_SCANCODE_A]) {
		camera.ProcessKeyboard(MOVE_LEFT, deltaTime);
	}
	// If key is D, then move to the right
	if (keystates[SDL_SCANCODE_D]) {
		camera.ProcessKeyboard(MOVE_RIGHT, deltaTime);
	}

	/// Change Camera Angles
	// If key is right arrow, then turn right
	if (keystates[SDL_SCANCODE_RIGHT]) {
		camera.ProcessKeyboard(TURN_RIGHT, deltaTime);
	}
	// If key is left arrow, then turn left
	if (keystates[SDL_SCANCODE_LEFT]) {
		camera.ProcessKeyboard(TURN_LEFT, deltaTime);
	}
	// If key is up arrow, then turn up
	if (keystates[SDL_SCANCODE_UP]) {
		camera.ProcessKeyboard(TURN_UP, deltaTime);
	}
	// If key is down arrow, then turn down
	if (keystates[SDL_SCANCODE_DOWN]) {
		camera.ProcessKeyboard(TURN_DOWN, deltaTime);
	}

	/// Change Camera Zoom
	// If key is E, then zoom in
	if (keystates[SDL_SCANCODE_E]) {
		camera.ProcessKeyboard(ZOOM_IN, deltaTime);
	}
	// If key is Q, then zoom out
	if (keystates[SDL_SCANCODE_Q]) {
		camera.ProcessKeyboard(ZOOM_OUT, deltaTime);
	}

	return camera;
}



// Main
int main(int argc, char* args[]) {

	// Use OpenGL 3.3
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	
	/// Initialize Everything
	// Check if SDL_Init is okay. Should be equal to 0.
	if (SDL_Init(SDL_INIT_VIDEO) > 0) {
		printf("HEY.. SDL_Init HAS FAILED. SDL_ERROR: %s\n", SDL_GetError());
		//exit(1);
	}

	// Initialize SDL Mixer
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("SDL Mixer could not initialize! SDL Error: %s\n", SDL_GetError());
		//exit(1);
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		//exit(1);
	}


	/// Create Window
	int windowWidth = 960;
	int windowHeight = 540;
	float scale_factor = 3840 / windowWidth;

	// Creates a window with title, position (currently undefined), a size (width and height), and will be shown
	SDL_Window* gwindow = SDL_CreateWindow("sMaRT bRidGe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	// If window fails to create, then produce error message
	if (gwindow == NULL) {
		printf("Window failed to init. Error: %s\n", SDL_GetError());
		//exit(2);
	}

	// Create OpenGL context for window
	SDL_GLContext gContext = SDL_GL_CreateContext(gwindow);
	if (gContext == NULL) {
		printf("OpenGL context could not be created! Error: %s\n", SDL_GetError());
		//exit(3);
	}

	// Check if Glew can be initialized
	glewExperimental = GL_TRUE;  // Want the latest features of GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		// GLEW failed!
		printf("Glew_init has failed. Error: %s\n", glewGetErrorString(err));
		//exit(4);
	}

	// Use Vsync
	if (SDL_GL_SetSwapInterval(1) < 0) {
		printf("Warning: Unable to set Vsync! Error: %s\n", SDL_GetError());
		//exit(5);
	}

	// Set openGL viewport
	glViewport(0, 0, windowWidth, windowHeight);

	glEnable(GL_DEPTH_TEST);  // Enable depth testing with z buffers


	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);


	// Sensor positions
	vector<glm::vec3> sensor_pos_p;

	// West sensors
	sensor_pos_p.push_back(glm::vec3(7.3316, -2.2241, -0.2));
	sensor_pos_p.push_back(glm::vec3(5.215, -2.2241, -0.2));
	sensor_pos_p.push_back(glm::vec3(3.1434, -2.2241, -0.2));
	sensor_pos_p.push_back(glm::vec3(1.0121, -2.2241, -0.2));
	sensor_pos_p.push_back(glm::vec3(-1.0337, -2.2241, -0.2));
	sensor_pos_p.push_back(glm::vec3(-3.1213, -2.2241, -0.2));
	sensor_pos_p.push_back(glm::vec3(-5.2075, -2.2241, -0.2));
	sensor_pos_p.push_back(glm::vec3(-9.3968, -2.2241, -0.2));

	// Roof sensors
	sensor_pos_p.push_back(glm::vec3(7.3032, 0.16036, 3.70));
	sensor_pos_p.push_back(glm::vec3(5.1952, 0.16036, 3.70));
	sensor_pos_p.push_back(glm::vec3(3.1156, 0.16036, 3.70));
	sensor_pos_p.push_back(glm::vec3(1.0298, 0.16036, 3.70));
	sensor_pos_p.push_back(glm::vec3(-1.0592, 0.16036, 3.70));
	sensor_pos_p.push_back(glm::vec3(-3.15, 0.16036, 3.70));
	sensor_pos_p.push_back(glm::vec3(-5.2528, 0.16036, 3.70));
	sensor_pos_p.push_back(glm::vec3(-7.3376, 0.16036, 3.70));

	// East sensors
	sensor_pos_p.push_back(glm::vec3(9.5568, 2.3272, -0.2));
	sensor_pos_p.push_back(glm::vec3(7.2941, 2.3272, -0.2));
	sensor_pos_p.push_back(glm::vec3(5.1225, 2.3272, -0.2));
	sensor_pos_p.push_back(glm::vec3(2.9113, 2.3272, -0.2));
	sensor_pos_p.push_back(glm::vec3(0.98272, 2.3272, -0.2));
	sensor_pos_p.push_back(glm::vec3(-1.0439, 2.3272, -0.2));
	sensor_pos_p.push_back(glm::vec3(-3.1877, 2.3272, -0.2));
	sensor_pos_p.push_back(glm::vec3(-5.3186, 2.3272, -0.2));
	sensor_pos_p.push_back(glm::vec3(-7.4455, 2.3272, -0.2));


	// Load model
	Model ourModel("repos/bridge5.obj", sensor_pos_p);

	// States
	vector<State> states;		// States vector
	int num_states = 20;			// Number of states

	for (int i = 0; i < num_states; i++) {
		states.push_back(State(i, "repos"));
	}

	states[0].loadMaterialTextures("repos/main_menu.png", glm::vec2(0.0f, -0.75f), glm::vec2(1.0f, 0.25f));
	states[0].loadMaterialTextures("repos/idea_by_bolaji.png", glm::vec2(0.84375f, 0.9537f), glm::vec2(0.15625f, 0.046296f));
	// Educational
	states[0].addButton(1, glm::vec4(665.0f, 1785.0f, 430.0f, 150.0f) / scale_factor);
	states[0].addButton(2, glm::vec4(665.0f, 1975.0f, 430.0f, 150.0f) / scale_factor);
	states[0].addButton(3, glm::vec4(1175.0f, 1785.0f, 430.0f, 150.0f) / scale_factor);
	states[0].addButton(4, glm::vec4(1175.0f, 1975.0f, 430.0f, 150.0f) / scale_factor);
	// Games
	states[0].addButton(5, glm::vec4(1800.0f, 1785.0f, 430.0f, 150.0f) / scale_factor);
	//states[0].addButton(6, glm::vec4(1800.0f, 1975.0f, 430.0f, 150.0f) / scale_factor);
	//states[0].addButton(7, glm::vec4(2285.0f, 1785.0f, 430.0f, 150.0f) / scale_factor);
	//states[0].addButton(8, glm::vec4(2285.0f, 1975.0f, 430.0f, 150.0f) / scale_factor);
	states[0].addButton(9, glm::vec4(2760.0f, 1785.0f, 430.0f, 150.0f) / scale_factor);
	//states[0].addButton(10, glm::vec4(2760.0f, 1975.0f, 430.0f, 150.0f) / scale_factor);
	// Misc
	states[0].addButton(11, glm::vec4(3395.0f, 1825.0f, 380.0f, 135.0f) / scale_factor);
	states[0].addButton(12, glm::vec4(3395.0f, 1990.0f, 380.0f, 135.0f) / scale_factor);

	// Educational
	// SHM
	states[1].loadMaterialTextures("repos/shm_1.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[1].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);
	states[1].addButton(1, glm::vec4(590.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[1].addButton(13, glm::vec4(1235.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[1].addButton(14, glm::vec4(1885.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);

	states[13].loadMaterialTextures("repos/shm_2.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[13].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);
	states[13].addButton(1, glm::vec4(590.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[13].addButton(13, glm::vec4(1235.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[13].addButton(14, glm::vec4(1885.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);

	states[14].loadMaterialTextures("repos/shm_3.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[14].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);
	states[14].addButton(1, glm::vec4(590.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[14].addButton(13, glm::vec4(1235.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[14].addButton(14, glm::vec4(1885.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);

	// Structural Dynamics
	states[2].loadMaterialTextures("repos/dyn_1.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[2].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);
	states[2].addButton(2, glm::vec4(590.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[2].addButton(15, glm::vec4(1235.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[2].addButton(16, glm::vec4(1885.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);

	states[15].loadMaterialTextures("repos/dyn_2.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[15].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);
	states[15].addButton(2, glm::vec4(590.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[15].addButton(15, glm::vec4(1235.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[15].addButton(16, glm::vec4(1885.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);

	states[16].loadMaterialTextures("repos/dyn_3.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[16].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);
	states[16].addButton(2, glm::vec4(590.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[16].addButton(15, glm::vec4(1235.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);
	states[16].addButton(16, glm::vec4(1885.0f, 1920.0f, 535.0f, 190.0f) / scale_factor);

	// Computer Vision
	states[3].loadMaterialTextures("repos/cv.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[3].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);

	// Bridge Info
	states[4].loadMaterialTextures("repos/bridge_info.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[4].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);

	// Games
	// Ninja Sneak
	states[5].loadMaterialTextures("repos/ninja_sneak.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[5].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);

	// Squirrel Dash
	//states[9].loadMaterialTextures("repos/squirrel_dash.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	//states[9].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);

	// Misc
	// Team
	states[11].loadMaterialTextures("repos/team.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[11].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);
	// References
	states[12].loadMaterialTextures("repos/references.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	states[12].addButton(0, glm::vec4(65.0f, 1965.0f, 420.0f, 150.0f) / scale_factor);


	// Mix music
	Mix_Music* gMusic = Mix_LoadMUS("repos/nights_like_this.mp3");
	if (!gMusic) {
		printf("Mix_LoadMUS music Error: %s\n", Mix_GetError());
		//exit(1);
	}
	
	Mix_PlayMusic(gMusic, -1);
	Mix_VolumeMusic(MIX_MAX_VOLUME / 15);

	// Sound Effects
	Mix_Chunk* gTentacle = Mix_LoadWAV("repos/tentacle_flop.mp3");
	if (!gTentacle) {
		printf("Mix_LoadMUS drum Error: %s\n", Mix_GetError());
		//exit(1);
	}


	// Camera
	Camera camera(glm::vec3(0.0f, 0.0f, 50.0f));  // Create camera

	// Model Matrix (From object coords to world coords)
	glm::mat4 model = glm::mat4(1.0f);  // Model Matrix (From object coords to world coords)
	glm::mat4 view;						// View matrix
	glm::mat4 projection;				// Projection matrix
	

	// Make Shader Program
	Shader modelShader("repos/shaders/model_vshader.vs", "repos/shaders/model_fshader.fs");  // Create shader program
	Shader guiShader("repos/shaders/gui_vshader.vs", "repos/shaders/gui_fshader.fs");  // Create shader program

	// Wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	// Get max number of attributes
	//int nrAttributes;
	//glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	//printf("Maximum nr of vertex attributes supported: %d\n", nrAttributes);



	/// Main Game Loop
	int currState = 0;  // Current game state. -1 = quit.
	SDL_Event event;	// Person-computer interaction

	State temp_state = states[0];  // Set temp state to first state

	float deltaTime = 0.0f;  // Time between current frame and last frame
	float prevTime = 0.0f;   // Previous time
	float updateTime = -30.0f;  // Time since last update

	float fov = 45.0f;

	while (1) {

		// Calculate current time and delta time
		float currTime = SDL_GetTicks() * 0.001f;   // Current time in sec
		deltaTime = currTime - prevTime;			// Update delta time
		prevTime = currTime;						// Update last frame

		//printf("curr time: %f\n", currTime);


		// Check for input (single click / press)
		while (SDL_PollEvent(&event)) {
			//printf("Curr state: %d\n", currState);
			int temp_num;						// Temp number
			temp_num = processStateInput(event, temp_state);  // Process exiting the program
			//printf("temp state: %d\n", temp_num);
			// If temp num is == -2, then no event happened,
			// so if it's != -2, then an event happened
			if (temp_num != -2) {
				currState = temp_num;
				Mix_PlayChannel(-1, gTentacle, 0);
			}
		}

		// Check if we need to exit
		if (currState == -1) {
			break;
		}

		// Update temp state
		temp_state = states[currState];

		// Process Input for Camera
		camera = processCamInput(deltaTime, camera);

		// Rendering commands
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Only need model for main menu
		if (currState == 0) {
			// Use Model shader program and set model matrix
			modelShader.use();  // Now every shader and rendering call will use shaderProgram
			modelShader.setMat4("model", model);

			// Update View Matrix
			view = camera.GetViewMatrix();
			modelShader.setMat4("view", view);

			// Update Projection Matrix
			projection = glm::perspective(glm::radians(camera.Fov), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
			modelShader.setMat4("projection", projection);

			// Create data
			std::vector<float> data;
			for (int i = 0; i < 25; i++) {
				float temp_val = sin(currTime + i / 25.0f);
				data.push_back(temp_val);
				//printf("data: %f\n", data.at(i));
			}

			// Actually render
			int update_bool = 0;	// Do we need to update the mesh? 0 = no. 1 = yes

			// If over 30 sec has passed, then update mesh and updateTime
			if (currTime - updateTime > 30.0f) {
				update_bool = 1;
				updateTime = currTime;
			}

			ourModel.Draw(modelShader, data, update_bool);

			glUseProgram(0);  // Reset shader program

		}
		

		// Gui shader
		guiShader.use();

		temp_state.draw(guiShader);

		glUseProgram(0);  // Reset shader program



		// Swap buffer
		SDL_GL_SwapWindow(gwindow);

		// Delay time
		int frameTicks = SDL_GetTicks() * 0.001f - currTime;  // Amount of time to complete the frame

		// Amount of time that the frame should take.
		// If the game is running too fast, then let the game rest
		// 1000 / window refresh rate (aka fps) = ms per frame
		//if (frameTicks < 1000 / 60) {
		//	SDL_Delay(1000 / 60 - frameTicks);  // Delay game time (ms)
		//}

	}

	// De-allocate all resources (Like buffers, arrays, shaderProgram)
	modelShader.deleteProgram();		// Delete shader program
	guiShader.deleteProgram();		// Delete shader program
	ourModel.clearModel();			// Clear memory in model

	// Free window and quit SDL
	SDL_DestroyWindow(gwindow);	// Destroy window
	SDL_Quit();					// Quit SDL

	
	return 0;
}