#version 400

// We define our lighting in the fragment shader


// Define the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texcoord;

// Passed in parameters
uniform mat4 model, view, projection;
uniform uint colourmode;
uniform int emitmode;
uniform mat3 normalmatrix;
uniform vec4 lightpos;
uniform vec4 light2pos;

// This is the output vertex colour sent to the rasterizer
out vec4 fcolour;

out vec4 fposition;
out vec3 fnormal;

out mat3 fnormalmatrix;
out uint fcolourmode;
flat out int femitmode;
out vec4 flightpos;
out vec4 flight2pos;
out vec2 ftexcoord; //our UV
void main() {
	fposition = vec4(position, 1.0);	
	fnormal = normal;
	fcolour = colour;

	fcolourmode = colourmode;
	fnormalmatrix = normalmatrix;
	femitmode = emitmode;
	flightpos = lightpos;
	flight2pos = light2pos;
	ftexcoord = texcoord;

	// Vertex position
	gl_Position = (projection * view * model) * fposition; 
}

