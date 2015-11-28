//Minimal fragment shader

#version 420

in vec4 fcolour;
in vec3 normal_h;
in vec4 position_h;
//in uint colourmode_f;
in mat3 normalmatrix_f;
flat in int emitmode_f;
in vec4 lightpos_f;
in vec4 light2pos_f;

out vec4 outputColour;
uniform mat4 model, view, projection;

struct LightProperties {
	vec3 color;
	vec4 position;
};

const int maxLights = 2;

uniform LightProperties Lights[maxLights];

// Global constants (for this vertex shader)
vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec3 global_ambient = vec3(0.0, 0.0, 0.0);
int  shininess = 24;




void main()
{
	vec3 emissive = vec3(0);				// Create a vec3(0, 0, 0) for our emmissive light
	
	vec4 diffuse_albedo  = fcolour;
				

	vec3 ambient = diffuse_albedo.xyz * 0.2;

	vec4 finalColour = vec4(0);

	for (int i = 0; i < maxLights; i++) {
		vec3 light_pos3 = lightpos_f.xyz;
		vec3 light2_pos3 = light2pos_f.xyz;
		// Define our vectors to calculate diffuse and specular lighting
		mat4 mv_matrix = view * model;		// Calculate the model-view transformation
		vec4 P = mv_matrix * position_h;	// Modify the vertex position (x, y, z, w) by the model-view transformation
		vec3 N = normalize(normalmatrix_f * normal_h);		// Modify the normals by the normal-matrix (i.e. to model-view (or eye) coordinates )
		vec3 L = light_pos3 - P.xyz;		// Calculate the vector from the light position to the vertex in eye space
		float distanceToLight = length(L);	// For attenuation
		L = normalize(L);					// Normalise our light vector

		// Calculate the diffuse component
		vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo.xyz;

		// Calculate the specular component using Phong specular reflection
		vec3 V = normalize(-P.xyz);	
		vec3 R = reflect(-L, N);
		vec3 specular = pow(max(dot(R, V), 0.0), shininess) * specular_albedo;

		// Calculate the attenuation factor;
		float attenuation_k = 1.0;
		float attenuation = 1.0 / (0.4 + attenuation_k * pow(distanceToLight, 3));

		if (emitmode_f == 1) {
			emissive = vec3(1.0, 1.0, 0.8); 
		}
		else if (emitmode_f == 2) {
			emissive = vec3(0.0, 1.0, 0.8);
		}
		finalColour = vec4(attenuation*(ambient + diffuse + specular) + emissive + global_ambient, 1.0);
	}
	outputColour =  finalColour;

	
}