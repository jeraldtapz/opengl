#version 430 core

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
}; 

struct Material
{
	sampler2D diffuseTexture0;
	sampler2D specularTexture0;
	sampler2D normalTexture0;
	sampler2D maskTexture0;
	sampler2D shadowMap0;
	sampler2D heightTexture0;
	samplerCube reflectionTexture0;
	samplerCube diffIrradianceTexture0;

	vec3 specularColor;
	vec3 diffuseColor;

	float shininess;
}; // Structs


vec2 GetTexCoordWithOffset();
vec3 CalculateDirectionalLight(DirLight light, vec3 normal, vec3 fragToView, vec3 albedo, float metallic, float roughness, vec3 f0);
vec3 CalculatePointLightContrib(PointLight light, vec3 normal, vec3 fragToView, vec3 albedo, float metallic, float roughness, vec3 f0);
vec3 CalculateSpotLightContrib(SpotLight light, vec3 normal, vec3 fragToView, vec3 albedo, float metallic, float roughness, vec3 f0);
vec3 CalculateAmbientDiffuse(samplerCube irradianceMap, vec3 normal, vec3 fragToView, vec3 albedo, vec3 f0, float ao, float roughness);
float CalculateDirectionalShadow(vec3 normal);
//float CalculatePointShadow();
//vec2 GetTexCoords(float useParallaxLocal);
float when_gt(float x, float y);

float DistributionGGX(vec3 normal, vec3 halfwayVector, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 normal, vec3 viewVector, vec3 lightVector, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 f0); // function decl
vec3 FresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness); // function decl


in vec2 TexCoord;
in vec4 FragPosDirLightSpace;

in vec3 ViewPosTangent;
in vec3 FragPosTangent;
in DirLight DirLightTangent;
in PointLight PointLightsTangent[4];
in SpotLight SpotLightTangent;
in mat3 TBN; //in


out vec4 FragColor;

uniform Material mat;
uniform samplerCube pointShadowMap;
uniform float isFlashlightOn;
uniform vec2 tiling;
uniform vec2 offset;
uniform float farPlane;
uniform float useShadow;
uniform float useNormalMaps;
uniform float useIBL;
uniform float useParallax; //uniforms

uniform samplerCube prefilter;
uniform sampler2D brdfLut;

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;

void main()
{
	vec2 coord = GetTexCoordWithOffset();
	vec3 normal = texture(mat.normalTexture0, coord).rgb;
	normal = normal * 2 - 1.0;
	normal = normalize(normal);

	vec3 fragToView = normalize(ViewPosTangent - FragPosTangent);

	


	vec3 albedo = texture(mat.diffuseTexture0, coord).rgb;
	vec3 mask = texture(mat.maskTexture0, coord).rgb;
	float metallic = mask.r;
	float roughness = mask.g;
	roughness = clamp(roughness, 0.01, 1);
	float ao = mask.b;
	
	vec3 f0 = vec3(0.04); // 0.04 is normally accepted for most dielectrics
	f0 = mix(f0, albedo, metallic);

	vec3 point_light_contrib = vec3(0);
	for(int i = 0; i < 4; i++)
	{
		point_light_contrib += CalculatePointLightContrib(PointLightsTangent[i], normal, fragToView, albedo, metallic, roughness, f0);
	}

	vec3 dirLightContrib = CalculateDirectionalLight(DirLightTangent,  normal, fragToView, albedo, metallic, roughness, f0);
	vec3 spotLightContrib = CalculateSpotLightContrib(SpotLightTangent, normal, fragToView, albedo, metallic, roughness, f0);
	float dirShadow = CalculateDirectionalShadow(normal);

	dirLightContrib *= (1 - dirShadow);

	spotLightContrib = max(spotLightContrib, vec3(0));

	spotLightContrib *= isFlashlightOn;

	vec3 ambient = CalculateAmbientDiffuse(mat.diffIrradianceTexture0, normal, fragToView, albedo, f0, ao, roughness) * useIBL;

	FragColor = vec4(point_light_contrib + dirLightContrib + ambient , 1.0);
}

vec2 GetTexCoordWithOffset()
{
	return vec2(TexCoord.x * tiling.x + offset.x, TexCoord.y * tiling.y + offset.y);
}

vec3 CalculatePointLightContrib(PointLight light, vec3 normal, vec3 fragToView, vec3 albedo, float metallic, float roughness, vec3 f0)
{
	//calculate radiance
	vec3 radiance = vec3(0);

	vec3 fragToLight = normalize(light.lightPos - FragPosTangent);
	vec3 halfwayVector = normalize(fragToLight + fragToView);

	float distance = length(light.lightPos - FragPosTangent);
	float attenuation = 1.0/(distance * distance);
	radiance = light.diffuseColor * light.diffuseIntensity * attenuation;

	//cook-torrance brdf
	float D = DistributionGGX(normal, halfwayVector, roughness);
	float G = GeometrySmith(normal, fragToView, fragToLight, roughness);
	vec3 F = FresnelSchlick(max(dot(halfwayVector, fragToView), 0.0), f0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic; // this is to take into account that metallic surfaces absorbs all refracted light

	vec3 numerator = D*F*G;
	float denominator = 4.0 * max(dot(normal, fragToView), 0.0) * max(dot(normal, fragToLight), 0.0) + 0.01;
	vec3 specular = numerator / denominator;

	float nDotL = max(dot(normal, fragToLight), 0.0);
	vec3 Lo = (kD * albedo / PI + specular) * radiance * nDotL; // BRDF * RADIANCE * NDOTL

	return Lo;
}

vec3 CalculateDirectionalLight(DirLight light, vec3 normal, vec3 fragToView, vec3 albedo, float metallic, float roughness, vec3 f0)
{
	//calculate radiance
	vec3 radiance = vec3(0);

	vec3 fragToLight = normalize(-light.lightDir);
	vec3 halfwayVector = normalize(fragToLight + fragToView);

	radiance = light.diffuseColor * light.diffuseIntensity;

	//cook-torrance brdf
	float D = DistributionGGX(normal, halfwayVector, roughness);
	float G = GeometrySmith(normal, fragToView, fragToLight, roughness);
	vec3 F = FresnelSchlick(max(dot(halfwayVector, fragToView), 0.0), f0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic; // this is to take into account that metallic surfaces absorbs all refracted light

	vec3 numerator = D*F*G;
	float denominator = 4.0 * max(dot(normal, fragToView), 0.0) * max(dot(normal, fragToLight), 0.0) + 0.001;
	vec3 specular = numerator / denominator;

	float nDotL = max(dot(normal, fragToLight), 0.0);
	vec3 Lo = (kD * albedo / PI + specular) * radiance * nDotL; // BRDF * RADIANCE * NDOTL

	return Lo;
}

vec3 CalculateSpotLightContrib(SpotLight light, vec3 normal, vec3 fragToView, vec3 albedo, float metallic, float roughness, vec3 f0)
{
	//calculate radiance
	vec3 radiance = vec3(0);

	vec3 fragToLight = normalize(light.lightPos - FragPosTangent);
	vec3 halfwayVector = normalize(fragToLight + fragToView);

	float theta = max(dot(normalize(-light.spotDirection), fragToLight), 0.0); 
	float epsilon = light.innerCutOffValue - light.cutOffValue;
	float intensity = clamp((theta - light.cutOffValue)/epsilon, 0.01f, 1.0f);

	radiance = light.diffuseColor * light.diffuseIntensity * intensity;

	//cook-torrance brdf
	float D = DistributionGGX(normal, halfwayVector, roughness);
	float G = GeometrySmith(normal, fragToView, fragToLight, roughness);
	vec3 F = FresnelSchlick(max(dot(halfwayVector, fragToView), 0.0), f0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic; // this is to take into account that metallic surfaces absorbs all refracted light

	vec3 numerator = D*F*G;
	float denominator = 4.0 * max(dot(normal, fragToView), 0.0) * max(dot(normal, fragToLight), 0.0) + 0.001;
	vec3 specular = numerator / denominator;

	float nDotL = max(dot(normal, fragToLight), 0.0);
	vec3 Lo = (kD * albedo / PI + specular) * radiance * nDotL; // BRDF * RADIANCE * NDOTL

	return Lo;
}

vec3 CalculateAmbientDiffuse(samplerCube irradianceMap, vec3 normal, vec3 fragToView, vec3 albedo, vec3 f0, float ao, float roughness)
{
	vec3 kS = FresnelSchlickRoughness(max(dot(normal, fragToView), 0.0), f0, roughness);
	vec3 kD = 1.0 - kS;

	vec3 irradiance =texture(irradianceMap, normal).rgb;
	vec3 diffuse = irradiance * albedo;

	vec3 refl = reflect(-fragToView, normal);

	vec3 prefilteredColor = textureLod(prefilter, refl, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 envBRDF = texture(brdfLut, vec2(max(dot(normal, fragToView), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

	vec3 ambient = ( kD * diffuse  + specular ) * ao;

	return ambient;
}

float DistributionGGX(vec3 normal, vec3 halfwayVector, float roughness)
{
	float a = roughness * roughness; // squaring gives better results
	float a2 = a*a;
	float nDotH = max(dot(normal, halfwayVector), 0.0);
	float nDotH2 = nDotH * nDotH;

	float num = a2;
	float denom = nDotH2 * (a2 - 1.0) + 1.0;
	denom = PI * denom * denom;

	return num/denom;
}

float GeometrySchlickGGX(float nDotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num = nDotV;
	float denom = nDotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 normal, vec3 viewVector, vec3 lightVector, float roughness)
{
	float nDotV = max(dot(normal, viewVector), 0.0);
	float nDotL = max(dot(normal, lightVector), 0.0);
	float ggx2 = GeometrySchlickGGX(nDotV, roughness);
	float ggx1 = GeometrySchlickGGX(nDotL, roughness);

	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 f0)
{
	return f0 + (1 - f0) * pow(1 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
	return f0 + (max(vec3(1.0 - roughness),  f0) - f0) * pow(1 - cosTheta, 5.0);
}



float CalculateDirectionalShadow(vec3 normal)
{
	vec3 lightSpacePosProj = FragPosDirLightSpace.xyz/FragPosDirLightSpace.w;
	lightSpacePosProj = lightSpacePosProj * 0.5 + 0.5;

	float closestDepth = texture(mat.shadowMap0, lightSpacePosProj.xy).r;
	float currentDepth = lightSpacePosProj.z;
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

float when_gt(float x, float y)
{
  return max(sign(x - y), 0.0f);
}
