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
layout(location = 1) in vec4 worldPos;
layout(location = 2) in vec4 posLightSpace[CASCADES_COUNT];

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
const float BIAS = 0.0005;

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

    float shadow = currentDepth - BIAS > closestDepth  ? 1.0 : 0.0;

	return shadow;
}

float depthDifference(vec4 shadowCoord, uint cascadeIndex)
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

	return (currentDepth - BIAS) - closestDepth;
}

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);


float rand(vec2 co){
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
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
		float cascadeCoef = 1.0;
		if(cascadeIndex > 0)
			cascadeCoef = 2.0;
		if(cascadeIndex > 2)
			cascadeCoef = 3.0;

		float distance = depthDifference(projCoords, cascadeIndex);
		float divisor = mix(1500.0, 100.0, distance * 10.0) * cascadeCoef;

		for(int i = 0; i < 16; i++)
		{
			int index = int(16.0*random(worldPos.xyz, i))%16;
			shadow += 1.0 - textureProj(projCoords + vec4(poissonDisk[index] / divisor, 0.0, 0.0), cascadeIndex);
		}
		shadow /= 16.0;
	}	

    outColor = vec4(shadow.rrr, 1.0);
}