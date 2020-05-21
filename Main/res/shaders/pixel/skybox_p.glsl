#version 330 core

out vec4 FragColor;

in vec3 TexCoord;
uniform samplerCube cubeMap;

void main()
{
	float gamma = 2.2f;
	FragColor = vec4(texture(cubeMap, TexCoord).rgb, 1.0);
	FragColor.rgb = pow(FragColor.rgb, vec3(gamma));
//	FragColor = vec4(0,0,1, 1.0);
}