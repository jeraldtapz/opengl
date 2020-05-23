#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 lightView[6];
uniform mat4 lightProj;

out vec4 FragPosWorldSpace;

void main()
{
	for(int i = 0; i < 6; i++)
	{
		gl_Layer = i;

		for(int j = 0; j < 3; j++)
		{
			FragPosWorldSpace = gl_in[j].gl_Position;
			gl_Position = lightProj * lightView[i] * FragPosWorldSpace;
			EmitVertex();
		}

		EndPrimitive();
	}
}