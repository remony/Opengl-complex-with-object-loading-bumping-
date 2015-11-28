/*


Modifications
- Loads in image texture
- No longer overwrites primary shaders; we undo glVertexAttribDivisor after writing the triangles
- in draw
	- we get the model pos (so the particle start is at the model pos)
	- modelrotation (so the particle shares the same rotation)
	- positionfix (allow you to move the particle start from the model pos)


*/

#include "DragonBreath.h"
#include <algorithm>
#include <glm/gtx/norm.hpp>
#include <soil\SOIL.h>

DragonBreath::DragonBreath()
{
	LastUsedParticle = 0;
}


DragonBreath::~DragonBreath()
{
}

void DragonBreath::Sort(){
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}


void DragonBreath::create(GLuint program)
{
	glGenVertexArrays(1, &ID.VertexArray);
	glBindVertexArray(ID.VertexArray);
	ID.program = program;
	glUseProgram(ID.program);

	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	static const GLfloat g_vertex_buffer_data[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};

	g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	g_particule_color_data = new GLubyte[MaxParticles * 4];

	for (int i = 0; i<MaxParticles; i++){
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	glGenBuffers(1, &buffer.vertex);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &buffer.position);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.position);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles

	glGenBuffers(1, &buffer.colour);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.colour);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Vertex shader
	GLuint CameraRight_worldspace_ID = glGetUniformLocation(ID.program, "CameraRight_worldspace");
	GLuint CameraUp_worldspace_ID = glGetUniformLocation(ID.program, "CameraUp_worldspace");
	GLuint ViewProjMatrixID = glGetUniformLocation(ID.program, "VP");

	/* load an image file directly as a new OpenGL texture */
	Texture = SOIL_load_OGL_texture("images/explosiontex2.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

	/* check for an error during the load process */
	if (Texture == 0)
	{
		printf("TexID SOIL loading error: '%s'\n", SOIL_last_result());
	}

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	/* Define the uniform variables */
	defineUniforms();

	lastTime = glfwGetTime();
}


/* Update the particle animation and draw the particles */
void DragonBreath::draw(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix, glm::vec3 modelpos, glm::vec3 modelrotation, glm::vec3 positionFix)
{
	double currentTime = glfwGetTime();
	double delta = currentTime - lastTime;
	lastTime = currentTime;

	if (loc != -1)
	{
		glUniform1f(loc, false);
	}
	if (loc2 != -1)
	{
		glUniform1f(loc2, false);
	}

	glBindVertexArray(ID.VertexArray);

	// We will need the camera's position in order to sort the particles
	// w.r.t the camera's distance
	glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);
	glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	// Generate 10 new particule each millisecond,
	// but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
	// newparticles will be huge and the next frame even longer.
	int newparticles = (int)(delta*1000.0);
	if (newparticles > (int)(0.016f*1000.0))
		newparticles = (int)(0.016f*1000.0);

	for (int i = 0; i<newparticles; i++){
		int particleIndex = findUnused();
		ParticlesContainer[particleIndex].life = 2.0f; // This particle will live 5 seconds.
		ParticlesContainer[particleIndex].pos = modelpos + positionFix;

		float spread = 2.5f;
		glm::vec3 maindir = modelrotation;
		// Very bad way to generate a random direction; 
		// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
		// combined with some user-controlled parameters (main direction, spread, etc)
		glm::vec3 randomdir = glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
			);

		ParticlesContainer[particleIndex].speed = maindir + randomdir*spread;

		// Very bad way to generate a random color
		ParticlesContainer[particleIndex].colour.r = 0;
		ParticlesContainer[particleIndex].colour.g = 0;
		ParticlesContainer[particleIndex].colour.b = 1;
		ParticlesContainer[particleIndex].colour.a = (rand() % 256) / 3;

		ParticlesContainer[particleIndex].size = 0.1;
	}

	// Simulate all particles
	int ParticlesCount = 0;
	for (int i = 0; i<MaxParticles; i++){

		Spark& p = ParticlesContainer[i]; // shortcut

		if (p.life > 0.0f){

			// Decrease life
			p.life -= delta;
			if (p.life > 0.0f){

				// Simulate simple physics : gravity only, no collisions
				p.speed += glm::vec3(0.0f, -0.1f, 0.0f) * (float)delta * 0.1f;
				p.pos += p.speed * (float)delta;
				p.cameradistance = glm::length2(p.pos - CameraPosition);
				//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

				// Fill the GPU buffer
				g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
				g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
				g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

				g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

				g_particule_color_data[4 * ParticlesCount + 0] = p.colour.r;
				g_particule_color_data[4 * ParticlesCount + 1] = p.colour.g;
				g_particule_color_data[4 * ParticlesCount + 2] = p.colour.b;
				g_particule_color_data[4 * ParticlesCount + 3] = p.colour.a;

			}
			else{
				// Particles that just died will be put at the end of the buffer in SortParticles();
				p.cameradistance = -1.0f;
			}

			ParticlesCount++;
		}
	}

	Sort();
	// Use our shader
	glUseProgram(ID.program);
	// Update the buffers that OpenGL uses for rendering.
	// There are much more sophisticated means to stream data from the CPU to the GPU, 
	// but this is outside the scope of this tutorial.
	// http://www.opengl.org/wiki/Buffer_Object_Streaming

	glBindBuffer(GL_ARRAY_BUFFER, buffer.position);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

	glBindBuffer(GL_ARRAY_BUFFER, buffer.colour);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	

	if (ID.Texture != -1)
	{
		glUniform1f(ID.Texture, true);
	}

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(ID.Texture, 0);

	// Same as the billboards tutorial
	glUniform3f(ID.CameraRight_worldspace, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
	glUniform3f(ID.CameraUp_worldspace, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

	glUniformMatrix4fv(ID.ViewProjMatrix, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vertex);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	// 2nd attribute buffer : positions of particles' centers
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.position);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : x + y + z + size => 4
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 3rd attribute buffer : particles' colors
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.colour);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : r + g + b + a => 4
		GL_UNSIGNED_BYTE,                 // type
		GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	glVertexAttribDivisor(0, 0); 
	glVertexAttribDivisor(1, 1); 
	glVertexAttribDivisor(2, 1); 

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}


void DragonBreath::defineUniforms()
{
	glBindVertexArray(ID.VertexArray);
	
	ID.CameraRight_worldspace = glGetUniformLocation(ID.program, "CameraRight_worldspace");
	ID.CameraUp_worldspace = glGetUniformLocation(ID.program, "CameraUp_worldspace");
	ID.ViewProjMatrix = glGetUniformLocation(ID.program, "VP");

	// fragment shader
	ID.Texture = glGetUniformLocation(ID.program, "usingtexture");
}


int DragonBreath::findUnused()
{
	for (int i = LastUsedParticle; i<MaxParticles; i++){
		if (ParticlesContainer[i].life < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i<LastUsedParticle; i++){
		if (ParticlesContainer[i].life < 0){
			LastUsedParticle = i;
			return i;
		}
	}
	return 0; // All particles are taken, override the first one
}