//Minimal fragment shader
#version 420

in vec4 fcolour;
in vec3 fnormal;
in vec4 fposition;
in mat3 fnormalmatrix;
in vec2 ftexcoord;
flat in int femitmode;
in vec4 flightpos;

out vec4 outputColour;

uniform sampler2D tex1;
uniform mat4 model, view, projection;

// Light Properties struct for storing data about lights
struct LightProperties {
	vec3 color;
	vec4 position;
};

// The limit of how many lights to have
const int maxLights = 2;
uniform LightProperties Lights[maxLights];

// Shadow colour applied with the specular
vec3 specular_albedo = vec3(1.0, 1, 1);

// Global colour override, this will change the colour of everything
vec3 global_ambient = vec3(0.2, 0.2, 0.2); 

// Define the shininess our shader will render things; more = darker but more shinyness; less = more light up models but less shinyness
int  shininess = 2;

struct Fog {
	float max_distance;
	float min_distance;
	vec4 colour;
};

Fog fog;


void main()
{
	// Assign the fog distances
	fog.max_distance = 30;
	fog.min_distance = 0;
	fog.colour = vec4(1,1,1, 1.0);


	// Create a vec3 of 0's
	vec3 emissive = vec3(0);
	
	// Set the light reflective colour to be the colour from the vertex shader
	vec4 light_albedo  = fcolour;
	vec3 ambient = light_albedo.xyz * 0.2;

	// Create an empty vec4 for the final lighting (for when we have multiple lights)  
	vec4 finalColour = vec4(0);
		
	// Calculate diffuse and specular lighting
	
	mat4 mv_matrix = view * model; // caculate the mv matrix
	vec4 Pos = mv_matrix * fposition;	// Vertex position

	
	vec3 Nor = normalize(fnormalmatrix * fnormal); // Modifying the normals by calculating the normals times the matrix normals
	vec3 L = flightpos.xyz - Pos.xyz; // Calculate the vector from the light position to the vertex in eye space

	float distanceToLight = length(L); // Calculate the distance

	// Normalised the light position after getting distance
	L = normalize(L);					

	// Calculate the diffuse
	
	//vec3 diffuse = max(0.0, dot(Nor, L))* light_albedo.xyz; // the max of the dot product of the normals and light times by the value of the light refraction xyz
	
	vec3 diffuse = (max(0.0, dot(Nor, L))* light_albedo.xyz) * light_albedo.xyz / distanceToLight;


	// Calculate Phong specular reflection
	vec3 V = normalize(-Pos.xyz); //Normalise the negative vertex position (negative: light colour infront; positive: light colour behind models)

	// Calculate the reflection of the negative distance and the combined normals
	vec3 R = reflect(-L, Nor);

	// Speculate calculation
	vec3 specular = pow(max(dot(R, V), 0.0), shininess) * specular_albedo; // the power of the dot product of the reflection and normalized vertex positions powered by the shininess factor defined in the global variable then times byt he reflection

	// Calculate the attenuation factor;
	float attenuation = 1.0 / (1.0 * (distanceToLight )+ 1.9f * pow(distanceToLight, 2)); 

	// Calculate our output colour
	finalColour = vec4(ambient + attenuation*(diffuse + specular) + emissive + global_ambient, 1.0);
	
	// Apply gamma correction
	finalColour = vec4 (
		pow(finalColour.x, 1.6),
		pow(finalColour.y, 1.6),
		pow(finalColour.z, 1.6),
		pow(finalColour.w, 1.0)
	);

	float fog_calc = 0;
	// Calculate the linear fog
	float FogDensity = 1;
	//float dist = length(L + fposition.xyz) - 0.9f;

	float fog_maxdist = 4.0;
float fog_mindist = 0;

	float dist = abs(L.z);
	//float fog_factor = (fog_maxdist-dist) / (fog_maxdist - fog_mindist);
	//fog_factor = clamp(fog_factor, 0.0, 1.0);

	fog_calc = 1.0 /exp( (dist * FogDensity)* (dist * FogDensity));
	 
	fog_calc = clamp( fog_calc, 0.0, 1.0 );


	// our output colour is a mix of the fog colour, lighting and fog coords
	
	
	
	vec4 texcolour = texture(tex1, ftexcoord);

	vec4 color = mix(fog.colour, finalColour, fog_calc);
	
	outputColour = finalColour * texcolour;
	
}