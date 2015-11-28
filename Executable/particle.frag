#version 330 core

in vec2 UV;
in vec4 colour;

out vec4 outputColor;

uniform sampler2D myTextureSampler;

void main(){
	if (colour.r < 0.1 && colour.g < 0.1 && colour.b < 0.1) {
		discard;
	}
	vec4 tex = texture2D( myTextureSampler, UV ) * colour;

	if (tex.r < 0.1 && tex.g < 0.1 && tex.b < 0.1) {
		discard;
	}
	outputColor = tex;
}
