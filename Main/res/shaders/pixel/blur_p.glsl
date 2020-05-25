#version 430 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D image;

uniform float isHorizontal;
float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
//float weight[7] = float[] (120, 560, 1820, 4368, 8008, 11440, 12870);


void main()
{
//	for(int i = 0; i < 7; i++)
//	{
//		weight[i]  /= 65502;
//	}

	vec2 texOffset = 1.0/textureSize(image, 0);
	vec3 color = texture(image, TexCoord).rgb * weight[0];

	vec3 horizontal, vertical;

	for(int i = 1; i < 5; i++)
	{
		horizontal += texture(image, TexCoord + vec2(texOffset.x * i, 0.0)).rgb * weight[i];
		horizontal += texture(image, TexCoord - vec2(texOffset.x * i, 0.0)).rgb * weight[i];
	}

	for(int i = 1; i < 5; i++)
	{
		vertical += texture(image, TexCoord + vec2(0, texOffset.y * i)).rgb * weight[i];
		vertical += texture(image, TexCoord - vec2(0, texOffset.y * i)).rgb * weight[i];
	}

	vec3 result = color;
	result += isHorizontal * horizontal + (1 - isHorizontal) * vertical;
	FragColor = vec4(result, 1.0);
}