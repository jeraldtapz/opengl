#version 330 core

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

float when_gt(float x, float y);
float CalculatePointShadow(vec3 worldPos, vec3 normal);
vec3 CalculatePointLight(vec3 normal, vec3 worldPos, vec4 diffSpec, PointLight light);


out vec4 FragColor;

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gDiffSpec;
uniform samplerCube pointShadowMap;

uniform PointLight pointLight;
uniform vec3 viewPos;
uniform float farPlane;
uniform float useShadow;
uniform float useDebug;


void main()
{
	vec2 coord = gl_FragCoord.xy/vec2(1600, 900);
	vec3 worldPos = texture(gPos, coord).rgb;
	vec3 normal = texture(gNormal, coord).rgb;
	vec4 diffSpec = texture(gDiffSpec, coord);

	float shadow = CalculatePointShadow(worldPos, normal);
	vec3 col = CalculatePointLight(normal, worldPos, diffSpec, pointLight);
	FragColor = (1 - useDebug) * vec4(col * (1 - shadow) , 1.0f) + vec4(pointLight.diffuseColor , 1) * useDebug;

	//tonemapping
//	vec3 c = FragColor.rgb;
//	vec3 hdrColor = c / (c + vec3(1));
//	hdrColor = vec3(1.0) - exp(-c * 1);
//	FragColor.rgb = hdrColor;
//
//	//temporary gamma correction
//	float gamma = 2.2;
//	FragColor = vec4(pow(FragColor.rgb, vec3(1.0/gamma)), FragColor.a);
}

vec3 CalculatePointLight(vec3 normal, vec3 worldPos, vec4 diffSpec, PointLight light)
{
	vec3 fragToLight = normalize(light.lightPos - worldPos);

	vec3 fragToView = normalize(viewPos - worldPos);
	vec3 reflectedLightDir = reflect(-fragToLight, normal);
	vec3 halfwayDir = normalize(fragToLight + fragToView);

	float distance = length(light.lightPos - worldPos);
	float attenuation = 1.0f /(1 + distance * light.linear + distance * distance * light.quadratic);

	float diffuseStrength = max(dot(normal, fragToLight), 0);
	vec3 diffuse = diffuseStrength * light.diffuseColor * light.diffuseIntensity;

	float specularStrength = pow(max(dot(halfwayDir, normal), 0.0), 32.0f);
	vec3 specular = specularStrength * light.specularColor * light.specularIntensity * 0;

	vec3 diffColor = diffuse * diffSpec.rgb;
	vec3 specColor = specular * diffSpec.a;

	vec3 result = diffColor + specColor; 
	result *= attenuation;
	return result;
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