#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoord;
layout(location = 5) in mat4 instanceModelMatrix;

layout(std140, binding = 1) uniform VP
{
	mat4 view;
	mat4 projection;
};

uniform mat4 model;
out vec2 TexCoord;

void main()
{
	TexCoord = aTexCoord;
	gl_Position = projection * view * instanceModelMatrix * vec4(aPos, 1.0);
}