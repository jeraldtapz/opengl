#version 330 core

float when_gt(float x, float y);
float CalculatePointShadow(vec3 worldPos, vec3 normal);

struct PointLight
{
	vec3 lightPos;
	vec3 diffuseColor;
	vec3 specularColor;
	float diffuseIntensity;
	float specularIntensity;

	float linear;
	float quadratic;
};

out vec4 FragColor;

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gDiffSpec;
uniform samplerCube pointShadowMap;

uniform PointLight pointLight;
uniform vec3 viewPos;
uniform float farPlane;
uniform float useShadow;


void main()
{
	vec2 coord = gl_FragCoord.xy/vec2(1280, 720);
	vec3 worldPos = texture(gPos, coord).rgb;
	vec3 normal = texture(gNormal, coord).rgb;
	vec4 diffSpec = texture(gDiffSpec, coord);
	float shadow = CalculatePointShadow(worldPos, normal);
	FragColor = vec4(diffSpec.rgb, 1.0f);

	//tonemapping
	vec3 c = FragColor.rgb;
	vec3 hdrColor = c / (c + vec3(1));
	hdrColor = vec3(1.0) - exp(-c * 1);
	FragColor.rgb = hdrColor;

	//temporary gamma correction
	float gamma = 2.2;
	FragColor = vec4(pow(FragColor.rgb, vec3(1.0/gamma)), FragColor.a);
}

float CalculatePointShadow(vec3 worldPos, vec3 normal)
{
	vec3 fragToLight =  (worldPos - pointLight.lightPos);
	
	float closestDepth = texture(pointShadowMap, fragToLight).r;
	closestDepth *= farPlane;

	float bias = max(0.05 * (1.0 - dot(normal, normalize(fragToLight))), 0.005);
	float currentDepth = length(fragToLight);
	return when_gt(currentDepth - bias, closestDepth);
}

float when_gt(float x, float y)
{
  return max(sign(x - y), 0.0f);
}