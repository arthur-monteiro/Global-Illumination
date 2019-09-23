#version 450
#extension GL_ARB_separate_shader_objects : enable

#define CASCADES_COUNT 4

layout(binding = 3) uniform UniformBufferObjectCSM
{
	float cascadeSplits[CASCADES_COUNT];
} uboCSM;

layout (binding = 4) uniform sampler2D shadowMap1;
layout (binding = 5) uniform sampler2D shadowMap2;
layout (binding = 6) uniform sampler2D shadowMap3;
layout (binding = 7) uniform sampler2D shadowMap4;

layout(location = 0) in vec3 viewPos;
layout(location = 1) in vec4 posLightSpace[CASCADES_COUNT];

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

float textureProj(vec4 shadowCoord, uint cascadeIndex)
{
	float closestDepth;
	if(cascadeIndex == 0)
		closestDepth = texture(shadowMap1, shadowCoord.st).r; 
	else if(cascadeIndex == 1)
		closestDepth = texture(shadowMap2, shadowCoord.st).r; 
	else if(cascadeIndex == 2)
		closestDepth = texture(shadowMap3, shadowCoord.st).r; 
	else if(cascadeIndex == 3)
		closestDepth = texture(shadowMap4, shadowCoord.st).r; 
    float currentDepth = shadowCoord.z;
	float bias = 0.0005;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

	return shadow;
}

vec2 poissonDisk[36] = vec2[](
	vec2(-0.412506, -0.505401),
	vec2(-0.198678, -0.362386),
	vec2(0.0612825, -0.360698),
	vec2(-0.680803, -0.662816),
	vec2(-0.275453, -0.057909),
	vec2(-0.57524, -0.225002),
	vec2(-0.135255, -0.644076),
	vec2(-0.357393, -0.873324),
	vec2(0.087715, -0.83279),
	vec2(0.274221, -0.575671),
	vec2(0.012977, 0.0652626),
	vec2(-0.908303, -0.306803),
	vec2(-0.632239, 0.236252),
	vec2(-0.972579, 0.0551293),
	vec2(-0.907004, 0.400391),
	vec2(0.26208, -0.129809),
	vec2(0.515364, -0.235511),
	vec2(0.470022, -0.743824),
	vec2(0.62804, -0.459096),
	vec2(0.332402, 0.200743),
	vec2(0.83457, -0.0398735),
	vec2(0.645883, 0.24368),
	vec2(0.367844, 0.453785),
	vec2(0.700306, 0.512157),
	vec2(-0.414574, 0.413026),
	vec2(-0.51206, 0.680345),
	vec2(0.40368, 0.800573),
	vec2(-0.0603344, 0.708574),
	vec2(0.0684587, 0.385982),
	vec2(-0.209951, 0.191194),
	vec2(-0.273511, 0.918154),
	vec2(0.19039, 0.661354),
	vec2(0.511419, 0.0151699),
	vec2(0.962834, -0.256675),
	vec2(0.897324, 0.331031),
	vec2(0.00145936, 0.959284)
);

float rand(vec2 co){
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
void main() 
{
	uint cascadeIndex = 0;
	for(uint i = 0; i < CASCADES_COUNT; ++i) 
	{
		if(viewPos.z <= uboCSM.cascadeSplits[i])
		{	
			cascadeIndex = i;
			break;
		}
	}

	vec4 projCoords = posLightSpace[cascadeIndex] / posLightSpace[cascadeIndex].w; 
	float shadow = 0;
	if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
	{
		outColor = vec4(0.0, 1.0, 0.0, 1.0);
		return;
	}
	else 
	{
		for(int i = 0; i < 16; i++)
		{
			int poissonDiskIndex = int(36.0*rand(gl_FragCoord.xy * i))%36;
			shadow += 1.0 - textureProj(projCoords / projCoords.w + vec4(poissonDisk[i] / 2000.0, 0.0, 0.0), cascadeIndex);
		}

		shadow /= 8;
	}	

    outColor = vec4(shadow.rrr, 1.0);
}