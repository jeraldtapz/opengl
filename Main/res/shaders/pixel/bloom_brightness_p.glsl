#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightnessColor;

float when_gt(float x, float y);

in vec2 TexCoord;

uniform sampler2D hdrColor;
uniform float brightnessThreshold;
uniform float useBloom;

void main()
{
	vec3 color = texture(hdrColor, TexCoord).rgb;
	float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float isBrightEnough = when_gt(brightness, brightnessThreshold);

	FragColor = vec4(color, 1.0) * useBloom;
	BrightnessColor = (isBrightEnough) * vec4(color, 1.0) + (1 - isBrightEnough) * vec4(0,0,0, 1.0);
	BrightnessColor = BrightnessColor * useBloom;
	FragColor = BrightnessColor;
}

float when_gt(float x, float y)
{
  return max(sign(x - y), 0.0f);
}