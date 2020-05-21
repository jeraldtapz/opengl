#version 330 core

in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenColor;

const float offset = 1.0 / 300.0; 


uniform float kernel[9];
uniform vec2 offsets[9];

//vec2 offsets[9] = vec2[](
//        vec2(-offset,  offset), // top-left
//        vec2( 0.0f,    offset), // top-center
//        vec2( offset,  offset), // top-right
//        vec2(-offset,  0.0f),   // center-left
//        vec2( 0.0f,    0.0f),   // center-center
//        vec2( offset,  0.0f),   // center-right
//        vec2(-offset, -offset), // bottom-left
//        vec2( 0.0f,   -offset), // bottom-center
//        vec2( offset, -offset)  // bottom-right    
//    );

//     float kernel[9] = float[](
//        -1, -1, -1,
//        -1,  9, -1,
//        -1, -1, -1
//    );

void main()
{
	vec3 samples[9];
	for(int i = 0; i < 9; i++)
	{
		samples[i] += vec3(texture(screenColor, TexCoord + offsets[i]));
	}
	vec3 color = vec3(0);
	for(int i = 0; i < 9; i++)
		color += samples[i] * kernel[i];

//	FragColor = vec4(color, 1);
	float gamma = 2.2;
	FragColor = vec4(texture(screenColor, TexCoord).rgb, 1);
	FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}