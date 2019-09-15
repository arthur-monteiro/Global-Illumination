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
		shadow = 1.0;
		cascadeIndex = -1;
	}
	else 
	{
		shadow = 1.0 - textureProj(projCoords / projCoords.w, cascadeIndex);
	}	

    outColor = vec4(shadow.rrr, 1.0);
}