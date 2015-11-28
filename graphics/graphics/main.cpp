/*
		Title: Small town and dragon

		Author: Stuart Douglas

		Note: Parts of this class contains code from Iain Martin examples from 2015



/*


/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "tinyobjloader.lib")
#pragma comment(lib, "soil.lib")

#include "wrapper_glfw.h"
#include <iostream>
#include <algorithm>
#include <soil\SOIL.h>
#include <thread>
#include "obj_loader.h"
#include "Terrain.h"
#include "Particle_effect.h"
#include "DragonBreath.h"
#include "fontObj.h"
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

GLuint drawmode;
GLuint program[1];
GLuint particle_program;
GLuint object_program;
GLuint snow_program;

GLuint vao;
GLuint selectedProgram;

fontObj fontobj;
GLuint particle_texID;
int currentTime;

Particle_effect particle_effect;
DragonBreath dragonBreath;

// Movement/Animation values
struct Movement {
	GLfloat angle_x;
	GLfloat angle_y;
	GLfloat angle_z;
	GLfloat angle_inc_x;
	GLfloat angle_inc_y;
	GLfloat angle_inc_z;
};

struct DeltaTime {
	double current;
	double last;
};

DeltaTime deltatime;
GLuint planeVertices;

// Create instance of movement
Movement movement;

// Model positioning values
GLfloat x, y, z;
GLfloat scale;

struct Rotation {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

struct model {
	glm::vec3 position;
	glm::quat orientation;
	Rotation rotation;
};

obj_loader object;
obj_loader dragon;
obj_loader house;
obj_loader building2;
obj_loader building3;
obj_loader building4;

obj_loader light_model;
Terrain *terrain;

model building1;

model dragon_model;
model house_model;
model building_model2;
model building_model3;
model building_model4;
model building_model5;
model building_model6;

GLuint lightSource1Vertices;

// Camera properties
struct Camera {
	Rotation pos;
	Rotation angle;
	GLfloat aspect_ratio;
	GLfloat radius;
};

// Create instance of camera struct
Camera camera;

struct LightProperties {
	Rotation pos;
};

LightProperties light;

/* Uniforms*/

GLuint modelID, viewID, projectionID;
GLuint lightposID;
GLuint normalmatrixID;
GLuint emitmodeID;

GLuint lightProperties;
GLuint emitmode;

// setDefaultValues intializing variables with there default values; this is used at start and whenever the user stats a reset
void setDefaultValues() {
	currentTime = 0;

	/*
		Default camera values
	*/

	camera.pos.x = 0.2;
	camera.pos.y = 1.1;
	camera.pos.z = 15.6;
	camera.angle.x = 8.6;
	camera.angle.y = 3;
	camera.angle.z = 0;
	camera.radius = 0;
	camera.aspect_ratio = 1024.f / 768.f;

	
	// Building 1 properties
	building1.position = glm::vec3(x - 2, y, z - 1);
	building1.rotation.x = 0;
	building1.rotation.y = 90.0;
	building1.rotation.z = 0;

	// House properties
	house_model.position = glm::vec3(x + 2, y, z - 1);
	house_model.rotation.x = 0;
	house_model.rotation.y = -90.0;
	house_model.rotation.z = 0;

	// Building 2
	building_model2.position = glm::vec3(x, y, z - 4);
	building_model2.rotation.x = 0;
	building_model2.rotation.y = -90.0;
	building_model2.rotation.z = 0;

	// Building 3
	building_model3.position = glm::vec3(x - 1.5, y, z + 3);
	building_model3.rotation.x = 0;
	building_model3.rotation.y = -90.0;
	building_model3.rotation.z = 0;

	// Building 4
	building_model4.position = glm::vec3(x -4, y, z - 1);
	building_model4.rotation.x = 0;
	building_model4.rotation.y = 0.0;
	building_model4.rotation.z = 0;

	// Building 5
	building_model5.position = glm::vec3(x +3, y, z - 3.5);
	building_model5.rotation.x = 0;
	building_model5.rotation.y = 0.0;
	building_model5.rotation.z = 0;

	// Building 6: the gingerbread house

	building_model6.position = glm::vec3(x + 3, y, z + 3);
	building_model5.rotation.x = 0;
	building_model5.rotation.y = 0;
	building_model5.rotation.z = 0;

	// Default coords for positioning
	x = 0;
	y = 3; 
	z = 0;

	// Dragon properties 

	dragon_model.position = glm::vec3(x + 5, y + 0.8, z);
	dragon_model.rotation.x = 0;
	dragon_model.rotation.y = 90.0;
	dragon_model.rotation.z = 0;

	// Default light properties
	light.pos.x = 3.55; 
	light.pos.y = 1.9;
	light.pos.z = 3.65;

	// Default movement/animation values
	movement.angle_x = movement.angle_y = movement.angle_z = 0;
	movement.angle_inc_x = movement.angle_inc_z = 0;
	movement.angle_inc_y = -1;
	scale = 1.f;
	emitmode = 0;
}

// printControls prints the user controls to the debug console.
void printControls() {

	std::cout << "               Controls" << std::endl;
	std::cout << "_______________________________________" << std::endl;
	std::cout << "|     Keys   |         Function       |" << std::endl;
	std::cout << "|_____________________________________|" << std::endl;
	std::cout << "|				Camera					|" << std::endl;
	std::cout << "|_____________________________________|" << std::endl;
	std::cout << "|    Q, E    | Speed up/down gears    |" << std::endl;
	std::cout << "|      R     | Reset Speed/position   |" << std::endl;
	std::cout << "|  Up arrow  |      Look Up           |" << std::endl;
	std::cout << "| Down arrow |      Look Down         |" << std::endl;
	std::cout << "| Left arrow |      Look Left         |" << std::endl;
	std::cout << "| Right arrow|      Look Right        |" << std::endl;
	std::cout << "|    - / _   |       Zoom Out         |" << std::endl;
	std::cout << "|    = / +   |       Zoom In          |" << std::endl;
	std::cout << "|      A     |       Pan Left         |" << std::endl;
	std::cout << "|      D     |       Pan Right        |" << std::endl;
	std::cout << "|      S     |       Pan Down         |" << std::endl;
	std::cout << "|      W     |       Pan Up           |" << std::endl;
	std::cout << "|_____________________________________|" << std::endl;
	std::cout << "|				Lights					|" << std::endl;
	std::cout << "|_____________________________________|" << std::endl;
	std::cout << "|      J     |       Pan Left         |" << std::endl;
	std::cout << "|      L     |       Pan Right        |" << std::endl;
	std::cout << "|      K     |       Pan Down         |" << std::endl;
	std::cout << "|      I     |       Pan Up           |" << std::endl;
	std::cout << "|_____________________________________|" << std::endl;
	std::cout << std::endl;

}

void updateConsole() {
	system("cls");
	printControls();
}

// Intitilization
void init(GLWrapper *glw) {
	updateConsole();
	setDefaultValues();
	selectedProgram = 0;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	try
	{
		//Load in the vertex and fragment shader
		program[0] = glw->LoadShader("mist.vert", "mist.frag");
		particle_program = glw->LoadShader("particle.vert", "particle.frag");
		snow_program = glw->LoadShader("snow.vert", "snow.frag");
		object_program = glw->LoadShader("object.vert", "object.frag");
	}
	catch (std::exception &e) // If we capture an error; file load failure
	{
		std::cout << "Caught exception: " << e.what() << std::endl;
		std::cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	for (int i = 0; i < (sizeof(program) / sizeof(*program)); i++) {
		modelID = glGetUniformLocation(program[i], "model");
		emitmodeID = glGetUniformLocation(program[i], "emitmode");
		lightProperties = glGetUniformLocation(program[i], "LightProperties");
		viewID = glGetUniformLocation(program[i], "view");
		projectionID = glGetUniformLocation(program[i], "projection");
		lightposID = glGetUniformLocation(program[i], "lightpos");
		normalmatrixID = glGetUniformLocation(program[i], "normalmatrix");
	}
	
	// Create the particles
	particle_effect.create(snow_program);
	dragonBreath.create(particle_program);

	fontobj.initText2D("ExportedFont.TGA", glw);

	// Load in objects
	
	dragon.load_obj(
		"objs/dragon2.obj", 
		"objs/dragon_blue_tex.BMP", 
		"objs/dragon_NM.BMP", 
		program[selectedProgram]
	);

	dragon.createObject();

	object.load_obj(
		"objs/apartmentB.obj", 
		"objs/apartmentB_tex.BMP", 
		"objs/apartmentB_NM.BMP", 
		program[selectedProgram]
	);

	object.createObject();
	
	building2.load_obj(
		"objs/apartmenthouseE.obj",
		"objs/apartmentE_tex.BMP",
		"objs/apartmentE_NM.BMP",
		program[selectedProgram]
		);

	building2.createObject();

	building3.load_obj(
		"objs/apartmenthouseA.obj",
		"objs/apartmenthouseA_tex.BMP",
		"objs/apartmenthouseA_NM.BMP",
		program[selectedProgram]
		);

	building3.createObject();

	building4.load_obj(
		"objs/gingerbreadhouse.obj",
		"objs/gingerbreadhouse_tex.BMP",
		"objs/gingerbreadhouse_NM.BMP",
		program[selectedProgram]
		);

	building4.createObject();

	light_model.load_obj(
		"objs/tablelamp.obj",
		"objs/tablelamptex.BMP",
		"objs/tablelamptex.BMP",
		program[selectedProgram]
		);

	light_model.createObject();

	terrain = new Terrain(1, 1.f, 1.f);
	terrain->createTerrain(2000, 2000, 200.f, 200.f, 0);
	terrain->createObject(program[selectedProgram]);
}

// display loop
void display() {
	deltatime.current = glfwGetTime();
	deltatime.last = deltatime.current;
	float deltaTime = (float)(currentTime - deltatime.last);
	
	if (deltatime.current >= 255) {
		deltatime.current = 0;
	} 
	
	deltatime.last = deltatime.current;

	/*
				Note: 
						Below is a simple implmentation start of a day/night cycle
		
	*/

	//std::cout << deltatime.current << std::endl;
	// Define the background colour;  note: black/white may hide points, best 0.2,0.2,0.2 for debugging
	/*
	std::cout << "Time: " << currentTime << std::endl;
	
	
	//currentTime++;
	if (currentTime > 0 && currentTime < 1800) {
		glClearColor(33.f / 255, 150.f / 255, 243.f / 255, 1.0f);
	}
	else if (currentTime > 1800 && currentTime < 3600) {
		glClearColor(38.f / 255, 50.f / 255, 56.f / 255, 1.0f);
	}
	else if (currentTime > 3600) {
		currentTime = 0;
	}
	*/


	glClearColor(33.f / 255, 150.f / 255, 243.f / 255, 1.0f);

	// Clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(10);
	// Enable the Depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Assign the current shader application
	glUseProgram(program[selectedProgram]);

	// Intialize the default model transformations as a fallback
	glm::mat4 model = glm::mat4(1.0f);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(30.0f, camera.aspect_ratio, 0.1f, 100.0f);

	glm::vec3 headdirection = glm::vec3(0, 1, 0);
	
	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(camera.angle.x, camera.angle.y, camera.pos.z), // Camera is at (0,0,4), in World Space
		glm::vec3(camera.pos.x,camera.pos.y, camera.angle.z), // and looks at the origin
		headdirection // Head is up (set to 0,-1,0 to look upside-down)
	);

	//// Send our transformations to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &Projection[0][0]);

	// Define the light position and transform by the view matrix

	/*
				Light
	*/
	// Define the position in space
	glm::vec4 lightpos = View * glm::vec4(light.pos.x, light.pos.y, light.pos.z, 1.0);
	// Define the normal matrix
	glm::mat3 normalmatrix = glm::transpose(glm::inverse(glm::mat3(View * model)));

	// Send the light MVP to the vertex shader
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &Projection[0][0]);
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);	 // send the normal matrix
	glUniform4fv(lightposID, 1, glm::value_ptr(lightpos)); // send the light position to the vertex shader

	/*
			Drawing models

	*/

	model = glm::mat4(1.0f);
	model = glm::translate(model, building1.position);
	model = glm::scale(model, glm::vec3(scale * 0.2, scale * 0.2, scale * 0.2));
	model = glm::rotate(model, -movement.angle_x, glm::vec3(1, 0, 0)); 
	model = glm::rotate(model, building1.rotation.y, glm::vec3(0, 1, 0));
	model = glm::rotate(model, -building1.rotation.z, glm::vec3(0, 0, 1));

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	normalmatrix = glm::transpose(glm::inverse(glm::mat3(View * model)));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	object.drawObject(drawmode);

	model = glm::mat4(1.0f);
	model = glm::translate(model, building_model2.position);
	model = glm::scale(model, glm::vec3(scale * 0.2, scale * 0.2, scale * 0.2));
	model = glm::rotate(model, -movement.angle_x, glm::vec3(1, 0, 0)); 
	model = glm::rotate(model, building_model2.rotation.y, glm::vec3(0, 1, 0));
	model = glm::rotate(model, -building_model2.rotation.z, glm::vec3(0, 0, 1)); 

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	normalmatrix = glm::transpose(glm::inverse(glm::mat3(View * model)));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	building2.drawObject(drawmode);

	model = glm::mat4(1.0f);
	model = glm::translate(model, building_model3.position);
	model = glm::scale(model, glm::vec3(scale * 0.2, scale * 0.2, scale * 0.2));
	model = glm::rotate(model, -movement.angle_x, glm::vec3(1, 0, 0)); 
	model = glm::rotate(model, building_model3.rotation.y, glm::vec3(0, 1, 0));
	model = glm::rotate(model, -building_model3.rotation.z, glm::vec3(0, 0, 1)); 

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	normalmatrix = glm::transpose(glm::inverse(glm::mat3(View * model)));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	building3.drawObject(drawmode);


	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(dragon_model.position));
	model = glm::scale(model, glm::vec3(scale * 0.1, scale * 0.1, scale * 0.1));

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	normalmatrix = glm::transpose(glm::inverse(glm::mat3(View * model)));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	dragon.drawObject(drawmode);


	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(light.pos.x, light.pos.y, light.pos.z));
	model = glm::scale(model, glm::vec3(scale * 0.1, scale * 0.1, scale * 0.1));

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	normalmatrix = glm::transpose(glm::inverse(glm::mat3(View * model)));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	light_model.drawObject(drawmode);

		
		
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(x, y-3, z-30));
	model = glm::scale(model, glm::vec3(scale, scale, scale ));

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	normalmatrix = glm::transpose(glm::inverse(glm::mat3(View * model)));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	terrain->drawObject(drawmode);

	particle_effect.drawParticles(Projection, View);
	
	glm::vec3 positionFix = glm::vec3(0, 0.2, 0.7);
	glm::vec3 particleRotation = glm::vec3(0.f, -10.0f, 10.0f);
	dragonBreath.draw(Projection, View, dragon_model.position, particleRotation, positionFix);

	glDisableVertexAttribArray(0);

	char text[256];
	sprintf(text, "Controls in console or controls.txt", glfwGetTime());
	fontobj.printText2D(text, 0, 0, 20);

	model = glm::mat4(1.0f);
	model = glm::translate(model, building_model4.position);
	model = glm::scale(model, glm::vec3(scale * 0.2, scale * 0.2, scale * 0.2));
	model = glm::rotate(model, -movement.angle_x, glm::vec3(1, 0, 0)); // x axis rotation
	model = glm::rotate(model, building_model4.rotation.y, glm::vec3(0, 1, 0));
	model = glm::rotate(model, -building_model4.rotation.z, glm::vec3(0, 0, 1)); // z axis rotation

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	normalmatrix = glm::transpose(glm::inverse(glm::mat3(View * model)));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	building2.drawObject(drawmode);

	model = glm::mat4(1.0f);
	model = glm::translate(model, building_model5.position);
	model = glm::scale(model, glm::vec3(scale * 0.2, scale * 0.2, scale * 0.2));
	model = glm::rotate(model, -movement.angle_x, glm::vec3(1, 0, 0)); // x axis rotation
	model = glm::rotate(model, building_model5.rotation.y, glm::vec3(0, 1, 0));
	model = glm::rotate(model, -building_model5.rotation.z, glm::vec3(0, 0, 1)); // z axis rotation

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	normalmatrix = glm::transpose(glm::inverse(glm::mat3(View * model)));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	building3.drawObject(drawmode);

	model = glm::mat4(1.0f);
	model = glm::translate(model, building_model6.position);
	model = glm::scale(model, glm::vec3(scale * 0.4, scale * 0.4, scale * 0.4));
	model = glm::rotate(model, -movement.angle_x, glm::vec3(1, 0, 0)); // x axis rotation
	model = glm::rotate(model, building_model6.rotation.y, glm::vec3(0, 1, 0));
	model = glm::rotate(model, -building_model6.rotation.z, glm::vec3(0, 0, 1)); // z axis rotation

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	normalmatrix = glm::transpose(glm::inverse(glm::mat3(View * model)));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	building4.drawObject(drawmode);

	glUseProgram(0);

	// modify movement/animation values
	movement.angle_x += movement.angle_inc_x;
	movement.angle_y += movement.angle_inc_y;
	movement.angle_z += movement.angle_inc_z;
}


// When error occurs
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

/*
Name: InputHandler
Description: This method handles all of repeated input handling methods. To encourage reusability
*/
static void inputHandler(int key) {
	/*
			Camera Movement

			We are also moving the light with the camera,
			however this can be moved to another key bind
			a future situation.
			*/

	// Zoom: This will move the camera closer or further away
	if (key == GLFW_KEY_EQUAL) {
		// limit the zoom
		//if (camera.pos_z > 0.5) camera.pos_z -= 0.1f; // Zoom In
		camera.pos.z -= 0.8f;
	}
	if (key == GLFW_KEY_MINUS) {
		// limit zoom out
		//if (camera.pos_z < 8) camera.pos_z += 0.1f; // Zoom Out
		camera.pos.z += 0.8f;
	}

	// Move camera right
	if (key == GLFW_KEY_RIGHT) {
		//camera.pos_x += 0.1f;
		camera.pos.x += 0.1f;

	}
	// Move camera left
	if (key == GLFW_KEY_LEFT) {
		camera.pos.x -= 0.1f;

	}

	// Move camera up
	if (key == GLFW_KEY_UP) {
		camera.pos.y += 0.1f;

	}
	// Move camera down
	if (key == GLFW_KEY_DOWN) {
		camera.pos.y -= 0.1f;

	}

	if (key == GLFW_KEY_W) {
		camera.angle.y -= 0.1f;
	}
	if (key == GLFW_KEY_S) {
		camera.angle.y += 0.1f;
	}

	if (key == GLFW_KEY_A) {
		camera.angle.x -= 0.1f;
	}

	if (key == GLFW_KEY_D) {

		camera.angle.x += 0.1;

	}

	// Reset speed and camera position
	if (key == GLFW_KEY_R) {
		setDefaultValues();
	}

	/*
		Light movement
	*/

	if (key == GLFW_KEY_J) {
		light.pos.x += 0.1f;
	}

	if (key == GLFW_KEY_L) {
		light.pos.x -= 0.1f;
	}

	if (key == GLFW_KEY_I) {
		light.pos.y += 0.1f;
	}

	if (key == GLFW_KEY_K) {
		light.pos.y -= 0.1f;
	}

	if (key == GLFW_KEY_U) {
		light.pos.z -= 0.1f;
	}

	if (key == GLFW_KEY_O) {
		light.pos.z += 0.1f;
	}

}

// key_callback is called when a keyboard event is captured
static void key_callback(GLFWwindow* window, int key, int s, int action, int mods)
{
	// If the escape key is pressed, clost the window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// If the callback has action pressed
	if (action == GLFW_PRESS) {
		//TODO uncomment updateConsole in production
		updateConsole();
		inputHandler(key);
		/* Cycle between drawing vertices, mesh and filled polygons */
	}
	// If the callback has action repeated; if the user holds the button
	else if (action == GLFW_REPEAT) {
		inputHandler(key);
	}

	// Toggle between draw mode states;   0 = filled;   1 = lines;  2 = Points;
	if (key == 'N' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}
}

// reshape changes the view to fix the window
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	camera.aspect_ratio = float(w) / float(h);
}

int main(int argc, char* argv[]) {

	GLWrapper *glw = new GLWrapper(
		1024,				// Window width
		768,				// Window height
		"Snow, small town, dragon scene (View console for instructions and state information)"	// Window title
	);
	glw->setErrorCallback(error_callback);

	// If unable to load functions close the application
	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	// Print controls and info to the console
	updateConsole();

	// set the renderer
	glw->setRenderer(display);
	glw->setKeyCallback(key_callback); // Set callback for keyboard shortcuts
	glw->setReshapeCallback(reshape); // Set callback for handling window resizing

	//Initialize the glw wrapper
	init(glw);

	//Start the event loop
	glw->eventLoop();

	delete(glw); // detele the windows instance

	return 0;
}