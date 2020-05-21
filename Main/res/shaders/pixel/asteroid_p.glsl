
#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D diffuse;

void main()
{
	FragColor = vec4(texture(diffuse, TexCoord).rgb, 1.0);
}