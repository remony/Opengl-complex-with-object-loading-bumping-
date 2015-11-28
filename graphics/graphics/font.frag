

#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main(){

	vec4 newColour = texture2D( myTextureSampler, UV );   
   color = newColour;
	
	
}