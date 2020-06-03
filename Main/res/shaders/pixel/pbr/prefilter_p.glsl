#version 330 core

in vec3 WorldPos;
out vec4 FragColor;

uniform float roughness;
uniform samplerCube environmentMap;

const float PI = 3.14159265359;

float WhenGreaterThan(float x, float y);
float WhenLessThan(float x, float y);
float WhenEqual(float x, float y);

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

//function that returns a low discrepancy sequence
//N is sample size, i is sample number
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

//returns a sample vector that takes into account the specular lobe(based on roughness) and uses a low discrepancy sample
//N is the position of the fragment of the cube, which is also the normal of the "sphere" where we initially generate coordinates for
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;

    //get spherical coordinate of the sample that is influenced by the low discrepancy sample vec2
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y)/(1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    //transform from spherical to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    float isLess = WhenLessThan(abs(N.z), 0.999);
    vec3 up =  isLess * vec3(0.0, 0.0, 1.0) + (1 - isLess) * (1.0, 0.0, 0.0); 
    up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 biTangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + biTangent * H.y + N * H.z;

    return normalize(sampleVec);
}

float DistributionGGX(float nDotH, float roughness)
{
	float a = roughness * roughness; // squaring gives better results
	float a2 = a*a;
	float nDotH2 = nDotH * nDotH;

	float num = a2;
	float denom = nDotH2 * (a2 - 1.0) + 1.0;
	denom = PI * denom * denom;

	return num/denom;
}

void main()
{
    //split sum approximation assumes that view direction and reflection is equal to N
    vec3 N = normalize(WorldPos);
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0;

    vec3 prefilteredColor = vec3(0);

    for(uint i = 0u; i < SAMPLE_COUNT; i++)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V,H) * H - V); // ?

        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        float D = DistributionGGX(NdotH, roughness);
        float pdf = ((D * NdotH)/(4.0 * HdotV)) + 0.0001;

        float resolution = 512;
        float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
        float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

        float mipLevel = (1 - WhenEqual(roughness, 0.0)) * 0.5 * log2(saSample/ saTexel);
        mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 

        float greaterThanZero = WhenGreaterThan(NdotL, 0);
        prefilteredColor += greaterThanZero * textureLod(environmentMap, L, mipLevel).rgb * NdotL;
        totalWeight += greaterThanZero * NdotL;
    }

    prefilteredColor = prefilteredColor / totalWeight;
    FragColor = vec4(prefilteredColor, 1.0);
}


float WhenGreaterThan(float x, float y)
{
  return max(sign(x - y), 0.0f);
}

float WhenLessThan(float x, float y)
{
  return max(sign(y - x), 0.0f);
}

float WhenEqual(float x, float y) 
{
  return 1.0 - abs(sign(x - y));
}