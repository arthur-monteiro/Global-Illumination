#version 450
#extension GL_ARB_separate_shader_objects : enable

#define CASCADES_COUNT 3

layout(binding = 3) uniform UniformBufferObjectLights
{
	vec4 camPos;
	
	vec4 dirDirLight;
	vec4 colorDirLight;

	float usePCF;
	float ambient;

	float cascadeSplits1;
	float cascadeSplits2;
	float cascadeSplits3;
} uboLights;

layout(binding = 4) uniform sampler2D texAlbedo;
layout(binding = 5) uniform sampler2D texNormal;
layout(binding = 6) uniform sampler2D texRoughness;
layout(binding = 7) uniform sampler2D texMetal;
layout(binding = 8) uniform sampler2D texAO;

layout (binding = 9) uniform sampler2D shadowMap1;
layout (binding = 10) uniform sampler2D shadowMap2;
layout (binding = 11) uniform sampler2D shadowMap3;

layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec3 viewPos;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in mat3 tbn;
layout(location = 7) in vec4 posLightSpace[CASCADES_COUNT];

layout(location = 0) out vec4 outColor;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

const float PI = 3.14159265359;

float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex)
{
	float closestDepth;
	if(cascadeIndex == 0)
		closestDepth = texture(shadowMap1, shadowCoord.st + offset).r; 
	else if(cascadeIndex == 1)
		closestDepth = texture(shadowMap2, shadowCoord.st + offset).r; 
	else if(cascadeIndex == 2)
		closestDepth = texture(shadowMap3, shadowCoord.st + offset).r; 
    float currentDepth = shadowCoord.z;
	float bias = 0.0005;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

	return shadow;
}

float filterPCF(vec4 sc, uint cascadeIndex)
{
	//ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.0;
	float dx = scale * 1.0 / float(2048);
	float dy = scale * 1.0 / float(2048);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y), cascadeIndex);
			count++;
		}
	
	}
	return shadowFactor / count;
}

void main() 
{
	uint cascadeIndex = 0;
	float splits[] = { uboLights.cascadeSplits1, uboLights.cascadeSplits2, uboLights.cascadeSplits3 };
	for(uint i = 0; i < CASCADES_COUNT; ++i) {
		if(viewPos.z < splits[i]) {	
			cascadeIndex = i;
			break;
		}
	}

	vec4 projCoords = posLightSpace[cascadeIndex] / posLightSpace[cascadeIndex].w; 
	if(projCoords.x < 0.0)
	{
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}
	if(projCoords.x > 1.0)
	{
		outColor = vec4(1.0);
		return;
	}
	if(projCoords.y < 0.0)
	{
		outColor = vec4(1.0, 1.0, 0.0, 1.0);
		return;
	}
	if(projCoords.y > 1.0)
	{
		outColor = vec4(0.0, 1.0, 1.0, 1.0);
		return;
	}
	float shadow = 0;
	if (uboLights.usePCF == 1.0) 
	{
		shadow = 1.0 - filterPCF(projCoords / projCoords.w, cascadeIndex);
	}
	else 
	{
		shadow = 1.0 - textureProj(projCoords / projCoords.w, vec2(0.0), cascadeIndex);
	}	

	vec3 albedo = pow(texture(texAlbedo, fragTexCoord).xyz, vec3(2.2));
	float roughness = texture(texRoughness, fragTexCoord).x;
	float metallic = texture(texMetal, fragTexCoord).x;
	float ao = texture(texAO, fragTexCoord).x;
	vec3 normal = (texture(texNormal, fragTexCoord).xyz * 2.0 - vec3(1.0)) * tbn;

	vec3 N = normalize(normal); 
    vec3 V = normalize(uboLights.camPos.xyz - worldPos);
	vec3 R = reflect(-V, N);  

	vec3 F0 = vec3(0.04); 
    F0 = mix(F0,albedo, metallic);
	
	vec3 Lo = vec3(0.0);
	
	// calculate per-light radiance
	vec3 L = normalize(-uboLights.dirDirLight.xyz);
	vec3 H = normalize(V + L);
	vec3 radiance     = uboLights.colorDirLight.xyz;        
	
	// cook-torrance brdf
	float NDF = DistributionGGX(N, H, roughness);        
	float G   = GeometrySmith(N, V, L, roughness);      
	vec3 F    = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, roughness);       
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	  
	
	vec3 nominator    = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0); 
	vec3 specular     = nominator / max(denominator, 0.001);
		
	// add to outgoing radiance Lo
	float NdotL = max(dot(N, L), 0.0);                
	Lo += (kD * albedo / PI + specular) * radiance * NdotL;

	vec3 ambient = vec3(uboLights.ambient) * albedo;

    vec3 color = ambient * (1.0 - shadow) + Lo * shadow;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

	/*color /= 2.0;
	if(cascadeIndex == 0)
		color += vec3(0.5, 0.0, 0.0);
	else if(cascadeIndex == 1)
		color += vec3(0.0, 0.5, 0.0);
	else if(cascadeIndex == 2)
		color += vec3(0.0, 0.0, 0.5);*/

    outColor = vec4(color, texture(texAlbedo, fragTexCoord).a);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}