#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

struct Material
{
	sampler2D diffuseTexture0;
	sampler2D specularTexture0;
	sampler2D normalTexture0;
	sampler2D reflectionTexture0;

	vec3 specularColor;
	vec3 diffuseColor;

	float shininess;
};

uniform vec3 tiling;
uniform Material mat;

void main()
{
	vec4 col = texture(mat.diffuseTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y));
	FragColor = col;
}