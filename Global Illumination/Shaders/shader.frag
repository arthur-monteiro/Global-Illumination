#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 2) uniform UniformBufferObjectLightsParams
{
	vec4 camPos;
	
	vec4 dirDirLight;
	vec4 colorDirLight;

	vec4 materialAlbedo;
	float materialRoughness;
	float materialMetallic;
	float usePCF;
} uboLightParams;

layout (binding = 3) uniform sampler2D shadowMap;

layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 posLightSpace;

layout(location = 0) out vec4 outColor;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

const float PI = 3.14159265359;

float textureProj(vec4 shadowCoord, vec2 offset)
{
	float closestDepth = texture(shadowMap, shadowCoord.st + offset).r; 
    float currentDepth = shadowCoord.z;
	float bias = 0.0005;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

	return shadow;
}

float filterPCF(vec4 sc)
{
	//ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.0;
	float dx = scale * 1.0 / float(8192);
	float dy = scale * 1.0 / float(8192);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 8;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

void main() 
{	
	float ao = 1.0;
	//vec3 normal = vec3(0.0, 0.0, 1.0) * tbn;

	vec4 projCoords = posLightSpace / posLightSpace.w;
	float shadow = 1.0 - ((uboLightParams.usePCF == 1.0) ? filterPCF(projCoords) : textureProj(projCoords, vec2(0.0)));
	/*if(shadow == 1.0)
	{
		outColor = vec4(vec3(0.0), 1.0);
		return;
	}*/

	vec3 N = normalize(normal); 
    vec3 V = normalize(uboLightParams.camPos.xyz - worldPos);
	vec3 R = reflect(-V, N);  

	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, uboLightParams.materialAlbedo.xyz, uboLightParams.materialMetallic);
	
	vec3 Lo = vec3(0.0);
	
	// calculate per-light radiance
	vec3 L = normalize(-uboLightParams.dirDirLight.xyz);
	vec3 H = normalize(V + L);
	vec3 radiance     = uboLightParams.colorDirLight.xyz;        
	
	// cook-torrance brdf
	float NDF = DistributionGGX(N, H, uboLightParams.materialRoughness);        
	float G   = GeometrySmith(N, V, L, uboLightParams.materialRoughness);      
	vec3 F    = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, uboLightParams.materialRoughness);       
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - uboLightParams.materialMetallic;	  
	
	vec3 nominator    = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0); 
	vec3 specular     = nominator / max(denominator, 0.001);
		
	// add to outgoing radiance Lo
	float NdotL = max(dot(N, L), 0.0);                
	Lo += (kD * uboLightParams.materialAlbedo.rgb / PI + specular) * radiance * NdotL;

	vec3 ambient = vec3(0.0);

    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color * shadow, 1.0);
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