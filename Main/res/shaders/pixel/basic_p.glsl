#version 430 core

#define NR_PT_LIGHTS 4

float when_gt(float x, float y);
float when_lt(float x, float y);

struct Material
{
	sampler2D diffuseTexture0;
	sampler2D specularTexture0;
	sampler2D normalTexture0;
	sampler2D shadowMap0;
	sampler2D heightTexture0;
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
vec2 GetTexCoords(float useParallaxLocal);

in DirLight DirLightTangent;
in PointLight PointLightsTangent[4];
in SpotLight SpotLightTangent;
in vec3 ViewPosTangent;
in vec3 FragPosTangent;
in vec2 TexCoord;
in vec3 NormalTangent;
in vec4 FragPosDirLightSpace;
in mat3 TBN;
out vec4 FragColor;


uniform Material mat;
uniform vec3 ambientColor;
uniform float time;
uniform float isFlashlightOn;
uniform vec2 tiling;
uniform vec2 offset;
uniform samplerCube pointShadowMap;
uniform float farPlane;
uniform float useShadow;
uniform float useNormalMaps;
uniform float useParallax;

void main()
{
	vec4 diffColor = texture(mat.diffuseTexture0, GetTexCoords(1.0));

	float pointShadow = CalculatePointShadow() * useShadow;
	float dirShadow = CalculateDirectionalShadow() * useShadow;

	vec3 pointLightContrib;

	for(int i = 0; i < NR_PT_LIGHTS; i++)
	{
		pointLightContrib += (1 - pointShadow * when_lt(i, 1)) * CalculatePointLight(PointLightsTangent[i]);
	}
	vec3 dirLightContrib = CalculateDirectionalLight();
	vec3 spotLightContrib = CalculateSpotLightContrib(SpotLightTangent) * isFlashlightOn;
	vec3 ambientContrib = diffColor.rgb * ambientColor;

	FragColor =  vec4(((1 - dirShadow) * dirLightContrib) + pointLightContrib + spotLightContrib + ambientContrib, diffColor.a);
}

vec3 CalculateDirectionalLight()
{
	vec3 normal = texture(mat.normalTexture0, GetTexCoords(1.0)).rgb;
	normal = normal * 2 - 1.0;

	normal = (useNormalMaps) * normalize(normal) + (1 - useNormalMaps) * normalize(NormalTangent);

	vec3 fragToLight = -DirLightTangent.lightDir;

	vec3 fragToView = normalize(ViewPosTangent - FragPosTangent);
	vec3 reflectedLightDir = reflect(DirLightTangent.lightDir, normal);
	vec3 halfwayDir = normalize(fragToView + fragToLight);

	float diffuseStrength = max(dot(normal, fragToLight), 0);
	vec3 diffuse = diffuseStrength * DirLightTangent.diffuseColor * DirLightTangent.diffuseIntensity * mat.diffuseColor;

	float specularStrength = pow(max(dot(normal, halfwayDir), 0), mat.shininess);
	vec3 specular = specularStrength * DirLightTangent.specularColor * DirLightTangent.specularIntensity;


	vec3 diffColor = texture(mat.diffuseTexture0, GetTexCoords(1.0)).rgb;
	float specColor = texture(mat.specularTexture0,  GetTexCoords(1.0)).r;

	vec3 result = diffColor * diffuse + specColor * specular; 

	return result;
} // Directional Light

vec3 CalculatePointLight(PointLight light)
{
	vec3 normal = texture(mat.normalTexture0, GetTexCoords(1.0)).rgb;
	normal = normal * 2 - 1.0;

	normal = (useNormalMaps) * normalize(normal) + (1 - useNormalMaps) * normalize(NormalTangent);

	vec3 fragToLight = normalize(light.lightPos - FragPosTangent);

	vec3 fragToView = normalize(ViewPosTangent - FragPosTangent);
	vec3 reflectedLightDir = reflect(-fragToLight, normal);
	vec3 halfwayDir = normalize(fragToLight + fragToView);

	float distance = length(light.lightPos - FragPosTangent);
	float attenuation = 1.0f /(1 + distance * light.linear + distance * distance * light.quadratic);

	float diffuseStrength = max(dot(normal, fragToLight), 0);
	vec3 diffuse = diffuseStrength * light.diffuseColor * light.diffuseIntensity * mat.diffuseColor;

	float specularStrength = pow(max(dot(halfwayDir, normal), 0.0), mat.shininess);
	vec3 specular = specularStrength * light.specularColor * light.specularIntensity * mat.specularColor;

	vec3 diffColor = texture(mat.diffuseTexture0, GetTexCoords(1.0)).rgb;
	float specColor = texture(mat.specularTexture0,  GetTexCoords(1.0)).r * 0;

	vec3 result = diffColor * diffuse + specColor * specular; 
	result *= attenuation;
	return result;
} // Point Light

vec3 CalculateSpotLightContrib(SpotLight light)
{
	vec3 normal = texture(mat.normalTexture0, GetTexCoords(1.0)).rgb;
	normal = normalize(normal * 2 - 1.0);

	vec3 fragToLight = normalize(light.lightPos - FragPosTangent);

	vec3 fragToView = normalize(ViewPosTangent - FragPosTangent);
	vec3 halfwayDir = normalize(fragToLight + fragToView);

	float diffuseStrength = max(dot(normal, fragToLight), 0);
	vec3 diffuse = diffuseStrength * light.diffuseColor * light.diffuseIntensity * mat.diffuseColor;

	float specularStrength = pow(max(dot(halfwayDir, normal), 0), mat.shininess);
	vec3 specular = specularStrength * light.specularColor * light.specularIntensity * mat.specularColor;

	vec3 diffColor = texture(mat.diffuseTexture0, GetTexCoords(1.0)).rgb;
	vec3 specColor = texture(mat.specularTexture0, GetTexCoords(1.0)).rgb;

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
	vec3 lightSpacePosProj = FragPosDirLightSpace.xyz/FragPosDirLightSpace.w;
	lightSpacePosProj = lightSpacePosProj * 0.5 + 0.5;

	float closestDepth = texture(mat.shadowMap0, lightSpacePosProj.xy).r;
	float currentDepth = lightSpacePosProj.z;

	vec3 normal = texture(mat.normalTexture0, GetTexCoords(1.0)).rgb;
	normal = normal * 2 - 1.0;

	normal = useNormalMaps * normalize(normal) + (1 - useNormalMaps) * normalize(NormalTangent);

	vec3 fragToLight = normalize(-DirLightTangent.lightDir);

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
	//convert back to world space because the shadow map was done in world space
	vec3 fragToLight = TBN * (FragPosTangent - PointLightsTangent[0].lightPos);
	

	float closestDepth = texture(pointShadowMap, fragToLight).r;
	closestDepth *= farPlane;

	vec3 normal = texture(mat.normalTexture0, GetTexCoords(0.0)).rgb;
	normal = normal * 2 - 1.0;

	normal = (useNormalMaps) * normalize(normal) + (1 - useNormalMaps) * normalize(NormalTangent);

	float bias = max(0.05 * (1.0 - dot(normal, normalize(fragToLight))), 0.005);
	float currentDepth = length(fragToLight);
	return when_gt(currentDepth - bias, closestDepth);
}

vec2 GetTexCoords(float useParallaxLocal)
{
	vec2 texCoord = vec2(TexCoord.x * tiling.x + offset.x , TexCoord.y * tiling.y + offset.y);
	vec3 fragToView = normalize(ViewPosTangent - FragPosTangent);

	//parallax mapping
	const float minLayers = 8.0;
	const float maxLayers = 64.0;
	float numLayers = mix(minLayers, maxLayers, max(dot(vec3(0.0, 0.0, 1.0), fragToView), 0.0));
	float heightScale = 0.05;

	float perLayerDepth = 1.0/numLayers;
	float currentLayerDepth = 0.0;
	vec2 p = fragToView.xy * heightScale;
	vec2 deltaTexCoord = p/numLayers;

	vec2 currentTexCoord = texCoord;
	float currentDepthMapValue = texture(mat.heightTexture0, currentTexCoord).r;

	while(currentLayerDepth < currentDepthMapValue)
	{
		currentTexCoord -= deltaTexCoord;
		currentDepthMapValue = texture(mat.heightTexture0, currentTexCoord).r;
		currentLayerDepth += perLayerDepth;
	}

	vec2 prevTexCoord = currentTexCoord + deltaTexCoord;
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(mat.heightTexture0, prevTexCoord).r - currentLayerDepth + perLayerDepth;

	float weight = afterDepth/(afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoord * weight + currentTexCoord * (1.0 - weight);
	return (useParallaxLocal * useParallax) * finalTexCoords + ((1 - (useParallaxLocal * useParallax)) * texCoord);
}

float when_gt(float x, float y)
{
  return max(sign(x - y), 0.0f);
}

float when_lt(float x, float y)
{
  return max(sign(y - x), 0.0f);
}
