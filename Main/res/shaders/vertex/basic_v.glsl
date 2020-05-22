#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

layout(std140, binding = 1) uniform VP
{
	mat4 view;
	mat4 projection;
};

uniform mat4 model;
uniform mat4 lightView;
uniform mat4 lightProjection;


void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
	Normal = mat3(transpose(inverse(model))) * aNormal;
	FragPos = vec3(model * vec4(aPos, 1.0));
	FragPosLightSpace = lightProjection * lightView * model * vec4(aPos, 1.0);
}