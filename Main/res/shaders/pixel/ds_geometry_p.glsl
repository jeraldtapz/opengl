#version 330 core

layout(location = 0) out vec3 gPos;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gDiffSpec;

vec2 GetTexCoords();
vec3 GetNormals();
vec4 GetDiffuseSpec();

in vec2 TexCoord;
in vec3 FragPosWorld;
in vec3 VertexNormalWorld;
in mat3 TBN;

struct Material
{
	sampler2D diffuseTexture0;
	sampler2D specularTexture0;
	sampler2D normalTexture0;
	sampler2D heightTexture0;
};

uniform float useNormalMaps;
uniform float useParallax;
uniform Material mat;
uniform vec3 viewPos;
uniform vec2 tiling;
uniform vec2 offset;


void main()
{
	gPos = FragPosWorld;
	gNormal = GetNormals();
	gDiffSpec = GetDiffuseSpec();
}

vec3 GetNormals()
{
	vec2 texCoord = GetTexCoords();
	vec3 norm = texture(mat.normalTexture0, texCoord).rgb;
	norm = norm * 2 - 1.0;
	norm = normalize(TBN * norm);
	norm = useNormalMaps * norm + (1 - useNormalMaps) * normalize(VertexNormalWorld);
	return norm;
}

vec4 GetDiffuseSpec()
{
	vec2 texCoord = GetTexCoords();
	vec4 diff_spec = texture(mat.diffuseTexture0, texCoord);
	diff_spec.a = texture(mat.specularTexture0, texCoord).r;
	return diff_spec;
}

vec2 GetTexCoords()
{
	vec2 texCoord = vec2(TexCoord.x * tiling.x + offset.x , TexCoord.y * tiling.y + offset.y);
	vec3 fragToView = TBN * normalize(viewPos - FragPosWorld); // convert to tangent space

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
	return (useParallax) * finalTexCoords + ((1 - useParallax) * texCoord);
}