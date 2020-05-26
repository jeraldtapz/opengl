#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout( location = 3) in vec3 aTangent;
layout( location = 4) in vec3 aBitangent;


layout(std140, binding = 1) uniform VP
{
	mat4 view;
	mat4 projection;
};

uniform mat4 model;

out vec3 FragPosWorld;
out vec3 VertexNormalWorld;
out vec2 TexCoord;
out mat3 TBN;

void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0);
	FragPosWorld = worldPos.xyz;

	VertexNormalWorld = transpose(inverse(mat3(model))) * aNormal;
	TexCoord = aTexCoord;
	
	gl_Position = projection * view * worldPos;

	vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
	vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));

	if (dot(cross(N, T), B) < 0.0)
		T = T * -1.0;

	TBN = mat3(T,B,N);
}

