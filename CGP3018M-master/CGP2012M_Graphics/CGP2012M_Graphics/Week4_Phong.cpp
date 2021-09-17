//#include <cstdio>
//#include <cstdlib>
#include <iostream>
#include <vector>

//include shape, shader header files
#include "GLerror.h"
#include "SDL_Start.h"
#include "Triangle_T.h"
#include "Circle.h"
#include "CircleTexture.h"
#include "Square.h"
#include "Camera.h"
#include "Cube.h"
#include "Model.h"
#include "ModelLoaderClass.h"
#ifndef TEXTURECLASS_H
#define TEXTURECLASS_H
#ifndef SHADERCLASS_H
#define SHADERCLASS_H

// // GLEW - OpenGL Extension Wrangler - http://glew.sourceforge.net/
// // NOTE: include before SDL.h
#ifndef GLEW_H
#define GLEW_H
//#include <GL/glew.h>
#include "windows.h"

// SDL - Simple DirectMedia Layer - https://www.libsdl.org/
#ifndef SDL_H
#define SDL_H
#include "SDL.h"
#include "SDL_image.h"
//#include "SDL_mixer.h"
//#include "SDL_ttf.h"

// // - OpenGL Mathematics - https://glm.g-truc.net/
#define GLM_FORCE_RADIANS // force glm to use radians
// // NOTE: must do before including GLM headers
// // NOTE: GLSL uses radians, so will do the same, for consistency
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//***************
//variables
SDL_Event event;
SDL_Window* win;
bool windowOpen = true;
bool isFullScreen = false;
float bubbleSpeed = -0.001f;
float radius;
//screen boundaries for collision tests
float bX_r = 2.0f;
float bX_l = -2.0f;
float bY_t = 1.0f;
float bY_b = -1.0f;
//screen centre
float centreX = 0.0f;
float centreY = 0.0f;
//window aspect/width/height
int w;
int h;
float aspect;
int left;
int newwidth;
int newheight;

//transform matrices
glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

glm::mat4 normalMatrix;

glm::mat4 translate;
glm::mat4 rotate;
glm::mat4 scale;
glm::mat4 backgroundTranslate;
glm::mat4 backgroundScale;
glm::vec3 b_scaleFactor;
glm::mat4 modelRotate;
glm::mat4 modelScale;
glm::mat4 modelTranslate;
glm::vec2 screenSize;
float angle = 0;

std::vector<CircleTexture> circles;

//create camera
Camera cam;
//loac camera variables
glm::vec3 camPos;
glm::vec3 camTarget;

bool flag = true;

glm::vec3 lightCol;
glm::vec3 lightPos;
float ambientIntensity;

//**************
//function prototypes
CircleTexture updatePositions(CircleTexture c);
void handleInput();

int main(int argc, char* argv[]) {
	//start and initialise SDL
	SDL_Start sdl;
	SDL_GLContext context = sdl.Init();
	win = sdl.win;

	SDL_GetWindowSize(win, &w, &h);
	glViewport(0, 0, w, h);
	aspect = (float)w / (float)h;

	//error class
	GLerror glerr;
	int errorLabel;

	//GLEW initialise
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	//register debug callback
	if (glDebugMessageCallback)
	{
		std::cout << "Registering OpenGL Debug callback function" << std::endl;
		glDebugMessageCallback(glerr.openglCallbackFunction, &errorLabel);
		glDebugMessageControl(GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			NULL,
			true);
	}

	//*****************************************************
	//OpenGL specific data

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	//objects
	//create background square
	Square background;
	//create model
	//could make this better by specifying the texture in this model header
	Model model;
	//create model loader
	ModelImport modelLoader;
	modelLoader.LoadOBJ2("..//..//Assets//Models//blenderSphere.obj", model.vertices, model.texCoords, model.normals, model.indices);

	errorLabel = 0;

	//*********************
	//create texture collection
	//create textures - space for 4, but only using 2
	Texture texArray[4];
	//background texture
	texArray[0].load("..//..//Assets//Textures//spaceBackground.png");
	texArray[0].setBuffers();
	texArray[1].load("..//..//Assets//Textures//Main_Texture.jpg");
	texArray[1].setBuffers();
	texArray[2].load("..//..//Assets//Textures//Second_Texture.jpg");
	texArray[2].setBuffers();
	texArray[3].load("..//..//Assets//Textures//Perlin_Noise.jpg");
	texArray[3].setBuffers();

	//set up another texture sampler for noise mask

	errorLabel = 2;

	//OpenGL buffers
	background.setBuffers();
	model.setBuffers();

	errorLabel = 3;
	//*****************************************
	//set uniform variables
	int transformLocation;
	int modelLocation;
	int viewLocation;
	int projectionLocation;
	int importModelLocation;
	int importViewLocation;
	int importProjectionLocation;
	int backgroundColourLocation;
	int ambientIntensityLocation;
	int modelColourLocation;
	int modelAmbientLocation;
	int modelLightPosLocation;
	int normalMatrixLocation;
	int viewPositionLocation;
	int timeLocation;
	int srLocation;


	GLuint currentTime = 0;
	GLuint lastTime = 0;
	GLuint elapsedTime = 0;
	GLuint deltaTime = 0;

	float screen[2] = { w, h };
	std::cout << w << " " << h << std::endl;

	//light colour initial setting
	lightCol = glm::vec3(1.0f, 0.9f, 0.95f);
	//light position
	lightPos = glm::vec3(-3.0f, 0.0, 2.0f);
	//light distance setting
	ambientIntensity = 1.0f;


	//initialise transform matrices 
	//orthographic (2D) projection
	//projectionMatrix = glm::ortho(0.0f, 4.0f, 0.0f, 3.0f, -1.0f, 100.0f);
	//perspective (3D) projection
	projectionMatrix = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);
	//initialise view matrix to '1'
	viewMatrix = glm::mat4(1.0f);

	backgroundScale = glm::mat4(1.0f);
	backgroundTranslate = glm::mat4(1.0f);
	modelScale = glm::mat4(1.0f);
	modelRotate = glm::mat4(1.0f);
	modelTranslate = glm::mat4(1.0f);

	//once only scale to background, and translate to centre
	b_scaleFactor = { 60.0f, 50.0f, 1.0f };
	backgroundScale = glm::scale(backgroundScale, glm::vec3(b_scaleFactor));
	backgroundTranslate = glm::translate(backgroundTranslate, glm::vec3(0.0f, 0.0f, -2.0f));

	//translate imported model?
	modelTranslate = glm::translate(modelTranslate, glm::vec3(0.0f, 0.0f, -1.0f));

	//once only scale to model
	modelScale = glm::scale(modelScale, glm::vec3(0.5f, 0.5f, 0.5f));
	errorLabel = 4;

	//*****************************
	//'game' loop
	while (windowOpen)
	{
		//*************************
		//****************************
		// OpenGL calls.

		glClearColor(1.0f, 1.0f, 1.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//process inputs
		handleInput();
		cam.updateCamera();

		//time
		currentTime = SDL_GetTicks();
		deltaTime = currentTime - lastTime;
		elapsedTime += deltaTime;
		lastTime = currentTime;

		//update camera matrix
		//camera only moves side to side, formards and backwards (no rotation)
		// set position, target, up direction
		//viewMatrix = glm::lookAt(glm::vec3(cam.camXPos, cam.camYPos, cam.camZPos), cam.cameraTarget, cam.cameraUp);


		errorLabel = 5;
		//background
		glUseProgram(background.shaderProgram);

		//set background lighting
		backgroundColourLocation = glGetUniformLocation(background.shaderProgram, "uLightColour");
		glProgramUniform3fv(background.shaderProgram, backgroundColourLocation, 1, glm::value_ptr(lightCol));
		//light distance
		ambientIntensityLocation = glGetUniformLocation(background.shaderProgram, "uAmbientIntensity");
		glProgramUniform1f(background.shaderProgram, ambientIntensityLocation, ambientIntensity);

		//set background image
		modelLocation = glGetUniformLocation(background.shaderProgram, "uModel");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(backgroundTranslate * backgroundScale));
		viewLocation = glGetUniformLocation(background.shaderProgram, "uView");
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(cam.viewMatrix));
		projectionLocation = glGetUniformLocation(background.shaderProgram, "uProjection");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		//time
		//timeLocation = glGetUniformLocation(background.shaderProgram, "uTime");
		//glProgramUniform1f(background.shaderProgram, timeLocation, currentTime);
		//screen resolution
		//srLocation = glGetUniformLocation(background.shaderProgram, "uSr");
		//glProgramUniform2fv(background.shaderProgram, srLocation,1, screen);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texArray[0].texture);
		background.render();


		//set .obj model
		glUseProgram(model.shaderProgram);

		//get time uniform from models shader program 
		timeLocation = glGetUniformLocation(model.shaderProgram, "uTime");
		float elapsed = ((float)elapsedTime);
		glProgramUniform1f(model.shaderProgram, timeLocation, elapsed);


		//set sphere lighting
		modelColourLocation = glGetUniformLocation(model.shaderProgram, "uLightColour");
		glProgramUniform3fv(model.shaderProgram, modelColourLocation, 1, glm::value_ptr(lightCol));
		//light position
		modelLightPosLocation = glGetUniformLocation(model.shaderProgram, "uLightPosition");
		glProgramUniform3fv(model.shaderProgram, modelLightPosLocation, 1, glm::value_ptr(lightPos));
		//light distance
		modelAmbientLocation = glGetUniformLocation(model.shaderProgram, "uAmbientIntensity");
		glProgramUniform1f(model.shaderProgram, modelAmbientLocation, ambientIntensity);
		//rotation
		modelRotate = glm::rotate(modelRotate, (float)deltaTime / 2000, glm::vec3(0.0f, 1.0f, 0.0f));
		importModelLocation = glGetUniformLocation(model.shaderProgram, "uModel");
		glUniformMatrix4fv(importModelLocation, 1, GL_FALSE, glm::value_ptr(modelTranslate * modelRotate * modelScale));
		importViewLocation = glGetUniformLocation(model.shaderProgram, "uView");
		glUniformMatrix4fv(importViewLocation, 1, GL_FALSE, glm::value_ptr(cam.viewMatrix));
		importProjectionLocation = glGetUniformLocation(model.shaderProgram, "uProjection");
		glUniformMatrix4fv(importProjectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		normalMatrix = (glm::mat3)glm::transpose(glm::inverse(modelTranslate * modelRotate * modelScale));
		//set the normalMatrix in the shaders
		normalMatrixLocation = glGetUniformLocation(model.shaderProgram, "uNormalMatrix");
		glUniformMatrix4fv(normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		//set view position for specular component - use the camera position
		viewPositionLocation = glGetUniformLocation(model.shaderProgram, "uViewPosition");
		glProgramUniform3fv(model.shaderProgram, viewPositionLocation, 1, glm::value_ptr(cam.cameraPosition));

		//textures passed here 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texArray[1].texture);

		//sets current texture sampler unit
		glActiveTexture(GL_TEXTURE1);
		//get back ID of uniform named in shader file
		GLint id = glGetUniformLocation(model.shaderProgram, "aTop");
		//set shader program value to shader unit (1)
		glUniform1i(id, 1);
		//bind shader unit to texture
		glBindTexture(GL_TEXTURE_2D, texArray[2].texture); //second texture

		//noise texture set up 
		glActiveTexture(GL_TEXTURE2);
		//gets back uniform ID named in shader file
		GLint id2 = glGetUniformLocation(model.shaderProgram, "noiseTex");
		//set shader program value to shader unit - in this case its (2)
		glUniform1i(id2, 2);
		//bind shader unit to texture
		glBindTexture(GL_TEXTURE_2D, texArray[3].texture); //noise mask

		//renders the sphere 
		model.render();

		//clear render state up again 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);

		//set to wireframe so we can see the circles
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		SDL_GL_SwapWindow(sdl.win);

	}//end loop

	//****************************
	// Once finished with OpenGL functions, the SDL_GLContext can be deleted.
	SDL_GL_DeleteContext(context);

	SDL_Quit();
	return 0;
}

void handleInput()
{
	//*****************************
		//SDL handled input
		//Any input to the program is done here

	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			windowOpen = false;
		}
		if (event.type == SDL_WINDOWEVENT)
		{
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
				std::cout << "Window resized w:" << w << " h:" << h << std::endl;
				SDL_GetWindowSize(win, &w, &h);
				newwidth = h * aspect;
				left = (w - newwidth) / 2;
				glViewport(left, 0, newwidth, h);
				break;


			default:
				break;
			}
		}

		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{


			case SDLK_w:
				//move camera 'forward' in the -ve z direction
				cam.camZPos -= cam.camSpeed;
				break;

			case SDLK_s:
				//move camera 'backwards' in +ve z direction
				cam.camZPos += cam.camSpeed;
				break;

			case SDLK_d:
				//move camera right
				//move camera target with position
				cam.camXPos -= cam.camSpeed;
				cam.camXTarget -= cam.camSpeed;
				break;
			case SDLK_RIGHT:
				//move camera right
				//move camera target with position
				cam.camXPos -= cam.camSpeed;
				cam.camXTarget -= cam.camSpeed;
				break;
			case SDLK_a:
				//move camera left
				//move camera target with position
				cam.camXPos += cam.camSpeed;
				cam.camXTarget += cam.camSpeed;
				break;
			case SDLK_LEFT:
				//move camera left
				//move camera target with position
				cam.camXPos += cam.camSpeed;
				cam.camXTarget += cam.camSpeed;
				break;

			case SDLK_DOWN:
				//move camera down
				cam.camYPos += cam.camSpeed;
				cam.camYTarget += cam.camSpeed;
				break;
			case SDLK_UP:
				//move camera up
				cam.camYPos -= cam.camSpeed;
				cam.camYTarget -= cam.camSpeed;
				break;
			case SDLK_1:
				//give red colour
				lightCol = glm::vec3(1.0f, 0.0f, 0.0f);
				break;
			case SDLK_2:
				//give green colour
				lightCol = glm::vec3(0.0f, 1.0f, 0.0f);
				break;
			case SDLK_3:
				//give blue colour 
				lightCol = glm::vec3(0.0f, 0.0f, 1.0f);
				break;
			case SDLK_0:
				//revert back to normal colour 
				lightCol = glm::vec3(1.0f, 1.0f, 1.0f);
				break;
			case SDLK_q:
				windowOpen = false;
				break;
			case SDLK_ESCAPE:
				windowOpen = false;
				break;
			}
		}
	}
}
#endif
#endif
#endif
#endif