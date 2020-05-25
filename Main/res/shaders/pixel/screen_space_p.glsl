#version 330 core

const float offset = 1.0 / 300.0; 


in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenColor;
uniform sampler2D bloomBlur;
uniform float useHDR;
uniform float useGammaCorrection;
uniform float useBloom;


uniform float kernel[9];
uniform vec2 offsets[9];
uniform float exposure;

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


	//hdr
	vec3 c = texture(screenColor, TexCoord).rgb;
	vec3 bloomColor = texture(bloomBlur, TexCoord).rgb;
	c += useBloom * bloomColor;

	//tonemapping
	vec3 hdrColor = c / (c + vec3(1));
	hdrColor = vec3(1.0) - exp(-c * exposure);
	hdrColor = useHDR * hdrColor + (1 - useHDR) * c;


	//gamma correction
	float gamma = 2.2;
	FragColor = useGammaCorrection * vec4(pow(hdrColor, vec3(1.0/gamma)), 1.0) +  (1 - useGammaCorrection) * vec4(hdrColor, 1.0);
}