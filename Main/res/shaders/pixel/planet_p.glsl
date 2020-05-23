#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

struct Material
{
	sampler2D diffuseTexture0;
	sampler2D specularTexture0;
	sampler2D normalTexture0;
	sampler2D shadowMap0;
	samplerCube reflectionTexture0;

	vec3 specularColor;
	vec3 diffuseColor;

	float shininess;
};


uniform Material mat;

void main()
{
	FragColor = texture(mat.diffuseTexture0, TexCoord);
}