#version 330 core

out vec4 FragColor;

in vec3 TexCoord;
uniform samplerCube cubeMap;
uniform float lod;

void main()
{
	float gamma = 2.2f;
//	FragColor = vec4(texture(cubeMap, TexCoord).rgb, 1.0);
	FragColor = vec4(textureLod(cubeMap, TexCoord, lod).rgb, 1.0);

	//we need to reverse gamma correct since skybox texture used is loaded in linear space
	//and there is a post process gamma correction
	FragColor.rgb = pow(FragColor.rgb, vec3(gamma));
}