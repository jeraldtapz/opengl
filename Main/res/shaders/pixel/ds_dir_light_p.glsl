#version 330 core

float when_gt(float x, float y);
float CalculateDirectionalShadow(vec3 fragPos, vec3 norm);
vec3 CalculateDirectionalLight(vec3 pos, vec3 normal, vec4 diffSpec);

struct DirLight
{
	vec3 lightDir;
	vec3 diffuseColor;
	vec3 specularColor;
	float diffuseIntensity;
	float specularIntensity;
};

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gDiffSpec;
uniform sampler2D shadowMap;

uniform DirLight dirLight;
uniform vec3 viewPos;
uniform mat4 lightView;
uniform mat4 lightProjection;
uniform float useShadow;

void main()
{
	vec3 worldPos = texture(gPos, TexCoord).rgb;
	vec3 worldNormal = texture(gNormal, TexCoord).rgb;
	vec4 diffSpec = texture(gDiffSpec, TexCoord);
	float shadow = CalculateDirectionalShadow(worldPos, worldNormal);

	vec3 color = CalculateDirectionalLight(worldPos, worldNormal, diffSpec);
//	color += vec3(0.1);
	FragColor = vec4(color * (1 - shadow), 1.0);

	//temporary gamma correction
	float gamma = 2.2;
	FragColor = vec4(pow(FragColor.rgb, vec3(1.0/gamma)), 1.0);
}

vec3 CalculateDirectionalLight(vec3 pos, vec3 normal, vec4 diffSpec)
{
	vec3 fragToLight = normalize(-dirLight.lightDir);

	vec3 fragToView = normalize(viewPos - pos);
	vec3 reflectedLightDir = reflect(normalize(dirLight.lightDir), normal);
	vec3 halfwayDir = normalize(fragToView + fragToLight);

	float diffuseStrength = max(dot(normal, fragToLight), 0);
	vec3 diffuse = diffuseStrength * dirLight.diffuseColor * dirLight.diffuseIntensity;

	float specularStrength = pow(max(dot(normal, halfwayDir), 0), 64.0f);
	vec3 specular = specularStrength * dirLight.specularColor * dirLight.specularIntensity;


	vec3 diffColor = diffSpec.rgb;
	vec3 specColor = specular * diffSpec.a;

	vec3 result = diffColor * diffuse + specColor * specular; 

	return result;
}

float CalculateDirectionalShadow(vec3 fragPos, vec3 norm)
{
	vec4 fragPosDirLightSpace = lightProjection * lightView * vec4(fragPos, 1.0);

	vec3 lightSpacePosProj = fragPosDirLightSpace.xyz/fragPosDirLightSpace.w;
	lightSpacePosProj = lightSpacePosProj * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, lightSpacePosProj.xy).r;
	float currentDepth = lightSpacePosProj.z;

	vec3 normal = normalize(norm);

	vec3 fragToLight = normalize(-dirLight.lightDir);

	float bias = max(0.0005 * (1.0 - dot(normal, fragToLight)), 0.00005);

	vec2 texelSize = 1.0/textureSize(shadowMap, 0);
	const int halfKernelWidth = 2;
	float shadow = 0;

	for(int i = -halfKernelWidth; i <= halfKernelWidth; i++)
	{
		for(int j = -halfKernelWidth; j <= halfKernelWidth; j++)
		{
			closestDepth = texture(shadowMap, lightSpacePosProj.xy + vec2(i, j) * texelSize).r;
			shadow += when_gt(currentDepth - bias, closestDepth);
		}
	}

	shadow /= ((halfKernelWidth * 2 + 1) * (halfKernelWidth * 2 + 1));
	return shadow * useShadow;
}

float when_gt(float x, float y)
{
  return max(sign(x - y), 0.0f);
}