/*

All code is from here http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-11-2d-text/

Slightly modified to work with project

*/

#pragma once
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")

#include "wrapper_glfw.h"

#include <iostream>
#include <vector>
/* GLM core */
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h> 
#include <time.h>
#include <algorithm>
#include <glm/gtx/norm.hpp>
#include <soil\SOIL.h>

class fontObj
{
public:
	fontObj();
	~fontObj();

	
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> UVs;

	unsigned int Text2DTextureID;
	unsigned int Text2DVertexBufferID;
	unsigned int Text2DUVBufferID;
	unsigned int Text2DShaderID;
	unsigned int Text2DUniformID;

	GLWrapper *glw;


	void initText2D(const char * texturePath, GLWrapper *glw);
	void printText2D(const char * text, int x, int y, int size);
	void cleanupText2D();
};

