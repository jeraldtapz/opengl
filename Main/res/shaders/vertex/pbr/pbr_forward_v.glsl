#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout( location = 3) in vec3 aTangent;
layout( location = 4) in vec3 aBitangent;

struct DirLight
{
	vec3 lightDir;
	vec3 diffuseColor;
	vec3 specularColor;
	float diffuseIntensity;
	float specularIntensity;
};

struct PointLight
{
	vec3 lightPos;
	vec3 diffuseColor;
	vec3 specularColor;
	float diffuseIntensity;
	float specularIntensity;

	float linear;
	float quadratic;
};

struct SpotLight
{
	vec3 lightPos;
	vec3 diffuseColor;
	vec3 specularColor;
	float diffuseIntensity;
	float specularIntensity;

	vec3 spotDirection;
	float cutOffValue;
	float innerCutOffValue;
}; // Structs

uniform float hack;
uniform vec3 viewPos;
uniform mat4 model;
uniform mat4 lightView;
uniform mat4 lightProjection;
uniform DirLight dirLight;
uniform SpotLight spotLight;
uniform PointLight pointLights[4];


out vec2 TexCoord;
out vec4 FragPosDirLightSpace;

out vec3 ViewPosTangent;
out vec3 FragPosTangent;
out DirLight DirLightTangent;
out PointLight PointLightsTangent[4];
out SpotLight SpotLightTangent;
out mat3 TBN; // needed to transform from tangent space point light position to world space in the fragment shader

layout(std140, binding = 1) uniform VP
{
	mat4 view;
	mat4 projection;
};

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	for(int i = 0; i < 4; i++)
	{
		gl_Position += vec4(hack * pointLights[i].lightPos, 0.0);
	}
	TexCoord = aTexCoord;

	vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
	vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));

	if (dot(cross(N, T), B) < 0.0)
		T = T * -1.0;

	TBN = mat3(T,B,N);
	mat3 inverseTBN = transpose(TBN);

	FragPosTangent = inverseTBN * vec3(model * vec4(aPos, 1.0)); // fragment position in tangent space
	FragPosDirLightSpace = lightProjection * lightView * model * vec4(aPos, 1.0); 
	// fragment position in directional light space for directional light shadow calculations

	ViewPosTangent = inverseTBN * viewPos;

	DirLightTangent = dirLight;
	DirLightTangent.lightDir = inverseTBN * DirLightTangent.lightDir; // convert from world space to tangent space

	for(int i = 0 ; i < 4; i++)
	{
		PointLightsTangent[i] = pointLights[i];
		PointLightsTangent[i].lightPos = inverseTBN * PointLightsTangent[i].lightPos; // convert from worldspace to tangent space
	}

	//convert from world space to tangent space
	SpotLightTangent = spotLight;
	SpotLightTangent.lightPos = inverseTBN * SpotLightTangent.lightPos;
	SpotLightTangent.spotDirection = inverseTBN * SpotLightTangent.spotDirection;
}