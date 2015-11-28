/*
		obj_loader

		Follows example however using library and converting the libary exports into the format we want
		
		Improvments
		- use the library format
		- improve import time (library is light, it should be fast)

*/

#include "obj_loader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

obj_loader::obj_loader()
{
	attribute_v_coord = 0;
	attribute_v_colour = 1;
	attribute_v_normal = 2;
	attribute_v_texcoord = 3;
}

obj_loader::~obj_loader()
{
}

void obj_loader::load_obj(const char* filename, const char* image, const char* normalsimage, GLuint iprogram) {
	std::string err;
	program = iprogram;

	bool ret = tinyobj::LoadObj(shapes, materials, err, filename);

	if (!err.empty()) { // `err` may contain warning message.
		std::cerr << err << std::endl;
	}

	if (!ret) {
		exit(1);
	}

	std::cout << "# of shapes    : " << shapes.size() << std::endl;
	std::cout << "# of materials : " << materials.size() << std::endl;

	object.buffer.vertices_size = 0;

	// Loop through each shape
	for (size_t i = 0; i < shapes.size(); i++) {
		object.buffer.vertices_size += sizeof(float)* shapes[i].mesh.positions.size();
		// For each indice push it into our elements store
		for (size_t f = 0; f < shapes[i].mesh.indices.size(); f++) {
			object.elements.push_back(shapes[i].mesh.indices[f]);
		}

		// For each position push into our vertices store
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
			object.vertices.push_back(glm::vec3(shapes[i].mesh.positions[3 * v + 0], shapes[i].mesh.positions[3 * v + 1], shapes[i].mesh.positions[3 * v + 2]));
		}

		// For each normal push into our normals store
		object.normals.resize(object.normals.size(), glm::vec3(0.0, 0.0, 0.0));
		for (size_t v = 0; v < shapes[i].mesh.normals.size() / 3; v++) {
			object.normals.push_back(glm::vec3(shapes[i].mesh.normals[3 * v + 0], shapes[i].mesh.normals[3 * v + 1], shapes[i].mesh.normals[3 * v + 2]));
		}

		// For each texture position push into our texture postitions store
		if (shapes[i].mesh.texcoords.size() > 0) {
			for (size_t v = 0; v < shapes[i].mesh.texcoords.size() / 2; v++) {
				object.texturesStore.push_back(glm::vec2(shapes[i].mesh.texcoords[2 * v], shapes[i].mesh.texcoords[2 * v + 1]));
			}
		}

		/*
					Calculating tangents
					code from 
					http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
		*/
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
			glm::vec3 & v0 = object.vertices[i + 0];
			glm::vec3 & v1 = object.vertices[i + 1];
			glm::vec3 & v2 = object.vertices[i + 2];

			// Shortcuts for UVs
			glm::vec2 & uv0 = object.texturesStore[i + 0];
			glm::vec2 & uv1 = object.texturesStore[i + 1];
			glm::vec2 & uv2 = object.texturesStore[i + 2];
			glm::vec3 deltaPos1 = v1 - v0;
			glm::vec3 deltaPos2 = v2 - v0;

			// UV delta
			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

			object.tangents.push_back(glm::vec3(tangent));
			object.tangents.push_back(glm::vec3(bitangent));
		}
	}

	// If the object does not have any normals generate our own
	if (object.normals.size() == 0) {
		std::cout << "Adding normals" << std::endl;
		object.normals.resize(object.normals.size(), glm::vec3(0.0, 0.0, 0.0));
		for (size_t i = 0; i < shapes.size(); i++) {
			for (size_t f = 0; f < shapes[i].mesh.positions.size(); f++) {
				object.normals.push_back(glm::vec3(1, 1, 0));
			}
		}
	}

	// When there are no colours set default colours
	if (object.colours.size() == 0) {
		object.colours.resize(object.colours.size(), glm::vec3(0.0, 0.0, 0.0));
		for (size_t i = 0; i < shapes.size(); i++) {
			for (size_t f = 0; f < shapes[i].mesh.positions.size(); f++) {
				object.colours.push_back(glm::vec3(1, 1, 1));
			}
		}
	}


	// Get our uniform positions
	loc = glGetUniformLocation(program, "usingtexture");
	loc2 = glGetUniformLocation(program, "tex2");
	usingTextureUniform = glGetUniformLocation(program, "usingTextureNormals");
	

	// Load in the texture 
	try
	{
		glActiveTexture(GL_TEXTURE0);
		/* load an image file directly as a new OpenGL texture */
		texID = SOIL_load_OGL_texture(image, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		/* check for an error during the load process */
		if (texID == 0)
		{
			printf("TexID SOIL loading error: '%s'\n", SOIL_last_result());
		}

		if (loc2 >= 0)
		{
			std::cout << "sending image to shader" << std::endl;

			glBindTexture(GL_TEXTURE_2D, texID);
			// Set our "myTextureSampler" sampler to user Texture Unit 0
			glUniform1i(loc2, 0);
		}
	}
	catch (std::exception &e)
	{
		printf("\nImage file loading failed.");
	}


	// Load in the bump map texture
	try
	{
		glActiveTexture(GL_TEXTURE1);
		/* load an image file directly as a new OpenGL texture */
		texID2 = SOIL_load_OGL_texture(normalsimage, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		/* check for an error during the load process */
		if (texID2 == 0)
		{
			printf("TexID SOIL loading error: '%s %s'\n", SOIL_last_result(), normalsimage);
			if (usingTextureUniform != -1)
			{
				std::cout << "disabling texture normals" << std::endl;
				glUniform1f(usingTextureUniform, false);
			}
		}
		else {
			if (usingTextureUniform != -1)
			{
				// Tell the shader that we have textures so it processes the texture instead of the colour
				std::cout << "enabled texture normals" << std::endl;
				glUniform1f(usingTextureUniform, true);
			}
		}
		if (loc2 >=0)
		{
			std::cout << "sending image to shader" << std::endl;

			glBindTexture(GL_TEXTURE_2D, texID2);
			// Set our "myTextureSampler" sampler to user Texture Unit 0
			glUniform1i(loc2, 1);
		}
	}
	catch (std::exception &e)
	{
		printf("\nImage file loading failed.");
	}
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	
}


/* Copy the vertices, normals and element indices into vertex buffers */
void obj_loader::createObject()
{
	// If we have vertices store them in the buffer
	if (object.vertices.size() > 0) {
		glGenBuffers(1, &object.vbo.mesh_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_vertices);
		glBufferData(GL_ARRAY_BUFFER, object.vertices.size() * sizeof(glm::vec3), &(object.vertices[0]), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// If we have normals store them in the buffer
	if (object.normals.size() > 0) {
		glGenBuffers(1, &object.vbo.mesh_normals);
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_normals);
		glBufferData(GL_ARRAY_BUFFER, object.normals.size() * sizeof(glm::vec3), &(object.normals[0]), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// If we have elements store them in the buffer
	if (object.elements.size() > 0) {
		glGenBuffers(1, &object.vbo.mesh_elements);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.vbo.mesh_elements);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, object.elements.size() * sizeof(GLushort), &(object.elements[0]), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// If we have colours store them in the buffer
	if (object.colours.size() > 0) {
		glGenBuffers(1, &object.vbo.mesh_colours);
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_colours);
		glBufferData(GL_ARRAY_BUFFER, object.colours.size()  * sizeof(glm::vec3), &(object.colours[0]), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	// If we have texture positions store them in the buffer
	if (object.texturesStore.size() > 0) {
		glGenBuffers(1, &object.vbo.mesh_textures);
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_textures);
		glBufferData(GL_ARRAY_BUFFER, object.texturesStore.size() * sizeof(glm::vec2), &(object.texturesStore[0]), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// If we have tangents store them in the buffer
	if (object.tangents.size() > 0) {
		glGenBuffers(1, &object.vbo.mesh_tangent);
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_tangent);
		glBufferData(GL_ARRAY_BUFFER, object.tangents.size() * sizeof(glm::vec3), &object.tangents[0], GL_STATIC_DRAW);
	}

	// If we have bi tangents store them in the buffer
	if (object.bi_tangents.size()) {
		glGenBuffers(1, &object.vbo.mesh_bit_tangent);
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_bit_tangent);
		glBufferData(GL_ARRAY_BUFFER, object.bi_tangents.size() * sizeof(glm::vec3), &object.bi_tangents[0], GL_STATIC_DRAW);
	}
}

// Draws the object
void obj_loader::drawObject(int drawmode)
{
	glUseProgram(program);
	int size = 0;

	if (object.vertices.size() > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_vertices);
		glVertexAttribPointer(
			attribute_v_coord,  // attribute index
			3,                  // number of elements per vertex, here (x,y,z)
			GL_FLOAT,           // the type of each element
			GL_FALSE,           // take our values as-is
			0,                  // no extra data between each position
			0                   // offset of first element
			);
	}

	if (object.normals.size() > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_normals);
		glVertexAttribPointer(
			attribute_v_normal, // attribute
			3,                  // number of elements per vertex, here (x,y,z)
			GL_FLOAT,           // the type of each element
			GL_FALSE,           // take our values as-is
			0,                  // no extra data between each position
			0                   // offset of first element
			);
	}

	if (object.colours.size() > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_colours);
		glVertexAttribPointer(
			attribute_v_colour, 
			3, 
			GL_FLOAT, 
			GL_FALSE, 
			0, 
			0
		);
		glEnableVertexAttribArray(1);
	}

	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);

	if (object.texturesStore.size() > 0)	{
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_textures);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			attribute_v_texcoord, // attribute
			2,                  // number of elements per vertex, here (x,y,z)
			GL_FLOAT,           // the type of each element
			GL_FALSE,           // take our values as-is
			0,                  // no extra data between each position
			(void*)0            // offset of first element
			);
	
	}



	// 4th attribute buffer : tangents
	if (object.tangents.size() > 0) {
		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_tangent);
		glVertexAttribPointer(
				4,                         
				3,                    
				GL_FLOAT,              
				GL_FALSE,              
				0,                      
				(void*)0 
		);
	}
	if (object.bi_tangents.size() > 0) {
		glEnableVertexAttribArray(5);
		glBindBuffer(GL_ARRAY_BUFFER, object.vbo.mesh_bit_tangent);
		glVertexAttribPointer(
			5,                                
			3,                                
			GL_FLOAT,                        
			GL_FALSE,                         
			0,
			(void*)0   
		);
		
	}
		         
	// Tell shader we are using textures 
	if (usingTextureUniform != -1)
	{
		glUniform1f(usingTextureUniform, true);
	}

	// Give our images to the shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glUniform1i(loc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texID2);
	glUniform1i(loc, 1);

	if (object.elements.size() > 0) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.vbo.mesh_elements);
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	}
	
	if (drawmode == 1) {
		glPointSize(3.f);
		glDrawElements(GL_POINTS, size / sizeof(GLushort), GL_UNSIGNED_SHORT, (void*)0);
	}
	else  {
		glDrawElements(GL_TRIANGLES, size / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	}
	
	glDisable(GL_TEXTURE_2D);
}