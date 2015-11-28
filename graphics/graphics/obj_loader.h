#pragma once

#include "wrapper_glfw.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <tinyobjloader\tiny_obj_loader.h>
#include <soil\SOIL.h>

struct VBO {
	GLuint mesh_vertices;
	GLuint mesh_normals;
	GLuint mesh_textures;
	GLuint mesh_elements;
	GLuint mesh_colours;
	GLuint mesh_tangent;
	GLuint mesh_bit_tangent;
};

struct ObjectBuffer {
	size_t vertices_size;
	size_t indices_size;
	size_t texcoords_size;
	size_t normals_size;
};

struct Object {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texturesStore;
	std::vector<glm::vec3> colours;
	std::vector<GLushort> elements;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bi_tangents;
	VBO vbo;
	ObjectBuffer buffer;
};



class obj_loader
{
public:
	obj_loader();
	~obj_loader();

	void load_obj(const char* filename, const char* image, const char* normalsimage, GLuint iprogram);
	void drawObject(int drawmode);
	void createObject();

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	GLuint texData;
	GLuint texID, texID2;
	GLuint texUniform;
	GLuint useTexUniform;

	GLint loc, loc1, loc2, usingTextureUniform;

	int program;

	Object object;
	GLuint textures;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colour;
	GLuint attribute_v_texcoord;
};

