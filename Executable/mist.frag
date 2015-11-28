#version 420

in vec4 fcolour;
in vec3 fnormal;
in vec4 fposition;
in mat3 fnormalmatrix;
in vec2 ftexcoord;
flat in int femitmode;
in vec4 flightpos;

out vec4 outputColour;

layout (binding=0) uniform sampler2D tex1;
layout (binding=1)uniform sampler2D tex2;

uniform bool usingtexture;
uniform bool usingTextureNormals;
uniform mat4 model, view, projection;

// Shadow colour applied with the specular
vec3 specular_albedo = vec3(0.4, 0.4, 0.4);

// Global colour override, this will change the colour of everything
vec3 global_ambient = vec3(0.2, 0.2, 0.2); 

// Define the shininess our shader will render things; more = darker but more shinyness; less = more light up models but less shinyness
float  shininess = 1;


void main()
{
	// Create a vec3 of 0's
	vec3 emissive = vec3(0);
	
	// Set the light reflective colour to be the colour from the vertex shader
	vec4 light_albedo  = fcolour;
	vec3 ambient = light_albedo.xyz * 0.3;

	// Create an empty vec4 for the final lighting (for when we have multiple lights)  
	vec4 finalColour = vec4(0);
		
	// Calculate diffuse and specular lighting
	
	mat4 mv_matrix = view * model; // caculate the mv matrix
	vec4 Pos = mv_matrix * fposition;	// Vertex position

	vec3 Nor = vec3(0);

	if (usingTextureNormals) {
		Nor = normalize(texture2D(tex2, ftexcoord.xy).rgb * 2.0 - 1.0);
	} else {
		Nor = normalize(fnormalmatrix * fnormal);
	}
	
	vec3 L = flightpos.xyz - Pos.xyz; // Calculate the vector from the light position to the vertex in eye space

	float distanceToLight = length(L); // Calculate the distance

	// Normalised the light position after getting distance
	L = normalize(L);					

	// Calculate the diffuse
	vec3 diffuse = (max(0.0, dot(Nor, L))* light_albedo.xyz) * light_albedo.xyz / distanceToLight;

	// Calculate Phong specular reflection
	vec3 V = normalize(-Pos.xyz); //Normalise the negative vertex position (negative: light colour infront; positive: light colour behind models)

	// Calculate the reflection of the negative distance and the combined normals
	vec3 R = reflect(-L, Nor);

	// Speculate calculation
	vec3 specular = pow(max(dot(R, V), 0.0), shininess) * specular_albedo; // the power of the dot product of the reflection and normalized vertex positions powered by the shininess factor defined in the global variable then times byt he reflection

	// Calculate the attenuation factor
	float attenuation = 1.0;

	// Calculate our output colour
	finalColour = vec4(ambient + attenuation*(diffuse + specular) + emissive + global_ambient, 1.0);

	// our output colour is a mix of the fog colour, lighting and fog coords
	vec4 texcolour = texture(tex1, ftexcoord);

	if (usingtexture) {
		if (usingTextureNormals) {
			outputColour = finalColour * texcolour;
		} else {
			outputColour = finalColour * texcolour; 
		}
	} else {
		outputColour = finalColour;
	}
}