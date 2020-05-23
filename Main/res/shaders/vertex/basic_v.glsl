#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout( location = 3) in vec3 aTangent;
layout( location = 4) in vec3 aBitangent;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;
out mat3 TBN;

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

	vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
	vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));

	if (dot(cross(N, T), B) < 0.0)
		T = T * -1.0;

	TBN = mat3(T, B, N);
}