#pragma once
#include "wrapper_glfw.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <glm/gtx/norm.hpp>
#include <soil\SOIL.h>
 
struct Spark {
	glm::vec3 pos, speed;
	glm::vec4 colour;
	float size, angle, weight, life, cameradistance;

	bool operator<(const Spark& that) const {
		return this->cameradistance > that.cameradistance;
	}
};

struct Buffer {
	GLuint vertex;
	GLuint position;
	GLuint colour;
};

struct ID {
	GLuint VertexArray;
	GLuint program;
	GLuint Texture;
	GLuint CameraRight_worldspace;
	GLuint CameraUp_worldspace;
	GLuint ViewProjMatrix;
	GLuint projection;
	GLuint view;
};

class DragonBreath
{
public:
	DragonBreath();
	~DragonBreath();

	void create(GLuint program);
	int findUnused();
	void Sort();
	void draw(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix, glm::vec3 modelpos, glm::vec3 modelrotation, glm::vec3 positionFix);
	void defineUniforms();

	Buffer buffer;
	ID ID;

	const int MaxParticles = 10000;
	Spark ParticlesContainer[10000];
	int LastUsedParticle;
	GLfloat* g_particule_position_size_data;
	GLubyte* g_particule_color_data;
	int ParticlesCount;
	double lastTime;

	GLuint loc, loc2;
	GLuint Texture;
};

