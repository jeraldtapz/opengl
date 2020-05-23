#version 330 core

in vec4 FragPosWorldSpace;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
	float lightDistance = length(FragPosWorldSpace.xyz - lightPos);

	lightDistance = lightDistance / farPlane;

	gl_FragDepth = lightDistance;
}