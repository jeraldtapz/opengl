#version 330 core

#define NR_PT_LIGHTS 4

float when_gt(float x, float y);
float when_lt(float x, float y);

struct Material
{
	sampler2D diffuseTexture0;
	sampler2D specularTexture0;
	sampler2D normalTexture0;
	sampler2D shadowMap0;
	samplerCube reflectionTexture0;

	vec3 specularColor;
	vec3 diffuseColor;

	float shininess;
};

struct Light
{
	vec3 lightPos;

	vec3 diffuseColor;
	vec3 specularColor;
	float diffuseIntensity;
	float specularIntensity;
};

struct DirLight
{
	vec3 lightDir;
	vec3 diffuseColor;
	vec3 specularColor;
	float diffuseIntensity;
	float specularIntensity;
};

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

struct SpotLight
{
	vec3 lightPos;
	vec3 diffuseColor;
	vec3 specularColor;
	float diffuseIntensity;
	float specularIntensity;

	vec3 spotDirection;
	float cutOffValue;
	float innerCutOffValue;
}; // Structs

vec3 CalculateDirectionalLight();
vec3 CalculatePointLight(PointLight light);
vec3 CalculateSpotLightContrib(SpotLight light);
vec3 CalculateReflectionContrib();
float CalculateDirectionalShadow();
float CalculatePointShadow();


in vec3 FragPos;
in vec2 TexCoord;
in vec3 Normal;
in mat3 TBN;
in vec4 FragPosLightSpace;
out vec4 FragColor;


uniform Material mat;
uniform DirLight dirLight;
uniform SpotLight spotLight;
uniform PointLight pointLights[NR_PT_LIGHTS];

uniform vec3 viewPos;
uniform vec3 ambientColor;
uniform float time;
uniform float isFlashlightOn;
uniform float shouldReceiveShadow;
uniform vec3 tiling;
uniform samplerCube pointShadowMap;
uniform float farPlane;

void main()
{
	vec4 diffColor = texture(mat.diffuseTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y));

	vec3 pointLightContrib;

	for(int i = 0; i < NR_PT_LIGHTS; i++)
	{
		pointLightContrib += CalculatePointLight(pointLights[i]);
	}
	vec3 dirLightContrib = CalculateDirectionalLight();
	vec3 spotLightContrib = CalculateSpotLightContrib(spotLight) * isFlashlightOn;
	vec3 ambientContrib = diffColor.rgb * ambientColor;
//	vec3 reflectionContrib = 0.25f * CalculateReflectionContrib();
	float pointShadow = CalculatePointShadow();
	float dirShadow = CalculateDirectionalShadow();

	vec3 fragToLight = FragPos - pointLights[0].lightPos;
	float closestDepth = texture(pointShadowMap, fragToLight).r;
//	closestDepth *= farPlane;

	FragColor =  vec4(((1 - pointShadow) * pointLightContrib + (1 - dirShadow) * dirLightContrib + spotLightContrib) + ambientContrib, diffColor.a);

//	FragColor =  vec4((1 - pointShadow) * (pointLightContrib + dirLightContrib + spotLightContrib) + ambientContrib, diffColor.a);
//	FragColor = vec4(vec3(closestDepth), 1.0);
}

vec3 CalculateDirectionalLight()
{
	vec3 normal = texture(mat.normalTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y)).rgb;
	normal = normal * 2 - 1.0;
	normal = normalize(TBN * normal);

	vec3 fragToLight = -dirLight.lightDir;
	normal = normalize(Normal);

	vec3 fragToView = normalize(viewPos - FragPos);
	vec3 reflectedLightDir = reflect(dirLight.lightDir, normal);
	vec3 halfwayDir = normalize(fragToView + fragToLight);

	float diffuseStrength = max(dot(normal, fragToLight), 0);
	vec3 diffuse = diffuseStrength * dirLight.diffuseColor * dirLight.diffuseIntensity * mat.diffuseColor;

	float specularStrength = pow(max(dot(normal, halfwayDir), 0), mat.shininess);
	vec3 specular = specularStrength * dirLight.specularColor * dirLight.specularIntensity * mat.specularColor;


	vec3 diffColor = texture(mat.diffuseTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y)).rgb;
	vec3 specColor = texture(mat.specularTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y)).rgb;

	vec3 result = diffColor * diffuse + specColor * specular; 

	return result;
} // Directional Light

vec3 CalculatePointLight(PointLight light)
{
	vec3 normal = texture(mat.normalTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y)).rgb;
	normal = normal * 2 - 1.0;
	normal = normalize(TBN * normal);
	normal = normalize(Normal);

	vec3 fragToLight = normalize(light.lightPos - FragPos);

	vec3 fragToView = normalize(viewPos - FragPos);
	vec3 reflectedLightDir = reflect(-fragToLight, normal);
	vec3 halfwayDir = normalize(fragToLight + fragToView);

	float distance = length(light.lightPos - FragPos);
	float attenuation = 1.0f /(1 + distance * light.linear + distance * distance * light.quadratic);

	float diffuseStrength = max(dot(normal, fragToLight), 0);
	vec3 diffuse = diffuseStrength * light.diffuseColor * light.diffuseIntensity * mat.diffuseColor;

	float specularStrength = pow(max(dot(halfwayDir, normal), 0.0), mat.shininess);
	vec3 specular = specularStrength * light.specularColor * light.specularIntensity * mat.specularColor;

	vec3 diffColor = texture(mat.diffuseTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y)).rgb;
	vec3 specColor = texture(mat.specularTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y)).rgb;

	vec3 result = diffColor * diffuse + specColor * specular; 
	result *= attenuation;
	return result;
} // Point Light

vec3 CalculateSpotLightContrib(SpotLight light)
{
	vec3 normal = texture(mat.normalTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y)).rgb;
	normal = normalize(normal * 2 - 1.0);
	normal = normalize(TBN * normal);
	normal = normalize(Normal);
	vec3 fragToLight = normalize(light.lightPos - FragPos);

	vec3 fragToView = normalize(viewPos - FragPos);
	vec3 reflectedLightDir = reflect(-fragToLight, normal);
	vec3 halfwayDir = normalize(fragToLight + fragToView);

	float diffuseStrength = max(dot(normal, fragToLight), 0);
	vec3 diffuse = diffuseStrength * light.diffuseColor * light.diffuseIntensity * mat.diffuseColor;

	float specularStrength = pow(max(dot(halfwayDir, normal), 0), mat.shininess);
	vec3 specular = specularStrength * light.specularColor * light.specularIntensity * mat.specularColor;

	vec3 diffColor = texture(mat.diffuseTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y)).rgb;
	vec3 specColor = texture(mat.specularTexture0, vec2(TexCoord.x * tiling.x, TexCoord.y * tiling.y)).rgb;

	float theta = dot(normalize(-light.spotDirection), fragToLight); 
	float epsilon = light.innerCutOffValue - light.cutOffValue;
	float intensity = clamp((theta - light.cutOffValue)/epsilon, 0.0f, 1.0f);

	vec3 result = diffColor * diffuse + specColor * specular; 
	result *= intensity;
	return result;
} // Spotlight

vec3 CalculateReflectionContrib()
{
//	vec3 incident = normalize(FragPos - viewPos);
//	vec3 reflected = reflect(incident, normalize(Normal));
//	return texture(mat.reflectionTexture0, reflected).rgb;
	return vec3(0);
}

float CalculateDirectionalShadow()
{
	vec3 lightSpacePosProj = FragPosLightSpace.xyz/FragPosLightSpace.w;
	lightSpacePosProj = lightSpacePosProj * 0.5 + 0.5;

	float closestDepth = texture(mat.shadowMap0, lightSpacePosProj.xy).r;
	float currentDepth = lightSpacePosProj.z;

	vec3 normal = texture(mat.normalTexture0, TexCoord).rgb;
	normal = normal * 2 - 1.0;
	normal = normalize(TBN * normal);
	normal = normalize(Normal);

	vec3 fragToLight = normalize(-dirLight.lightDir);

	float bias = max(0.0005 * (1.0 - dot(normal, fragToLight)), 0.00005);

	vec2 texelSize = 1.0/textureSize(mat.shadowMap0, 0);
	const int halfKernelWidth = 2;
	float shadow = 0;

	for(int i = -halfKernelWidth; i <= halfKernelWidth; i++)
	{
		for(int j = -halfKernelWidth; j <= halfKernelWidth; j++)
		{
			closestDepth = texture(mat.shadowMap0, lightSpacePosProj.xy + vec2(i, j) * texelSize).r;
			shadow += when_gt(currentDepth - bias, closestDepth);
		}
	}

	shadow /= ((halfKernelWidth * 2 + 1) * (halfKernelWidth * 2 + 1));

	return shadow;
}

float CalculatePointShadow()
{
	vec3 fragToLight = FragPos - pointLights[0].lightPos;
	float closestDepth = texture(pointShadowMap, fragToLight).r;
	closestDepth *= farPlane;

	vec3 normal = texture(mat.normalTexture0, TexCoord).rgb;
	normal = normal * 2 - 1.0;
	normal = normalize(TBN * normal);
	normal = normalize(Normal);

	float bias = max(0.05 * (1.0 - dot(normal, normalize(fragToLight))), 0.005);
	float currentDepth = length(fragToLight);
	return when_gt(currentDepth - bias, closestDepth);
}

float when_gt(float x, float y)
{
  return max(sign(x - y), 0.0f);
}

float when_lt(float x, float y)
{
  return max(sign(y - x), 0.0f);
}
