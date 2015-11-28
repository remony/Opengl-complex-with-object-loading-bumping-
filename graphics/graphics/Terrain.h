#pragma once
#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <soil\SOIL.h>

class Terrain
{
public:
	Terrain(int octaves, GLfloat freq, GLfloat scale);
	~Terrain();

	void calculateNoise();
	void createTerrain(GLuint xp, GLuint yp, GLfloat xs, GLfloat ys, GLfloat sl);
	void calculateNormals();
	void stretchToRange(GLfloat min, GLfloat max);
	void defineSea(GLfloat sealevel);
	void defineTexture();

	void createObject(GLuint iprogram);
	void drawObject(int drawmode);

	GLuint texID;
	GLuint program;
	GLuint loc, loc2;
	glm::vec3 *vertices;
	glm::vec3 *colours;
	glm::vec3 *normals;
	std::vector<GLuint> elements;
	GLfloat* noise;

	GLuint vbo_mesh_vertices;
	GLuint vbo_mesh_normals;
	GLuint vbo_mesh_colours;
	GLuint ibo_mesh_elements;
	GLuint attribute_v_coord;
	GLuint attribute_v_normal;

	GLuint xsize;
	GLuint zsize;
	GLfloat width;
	GLfloat height;
	GLuint perlin_octaves;
	GLfloat perlin_freq;
	GLfloat perlin_scale;
	GLfloat height_scale;
};

