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
	vec2( 0.143832, -0.141008 ),
	vec2( 0.344959, 0.293878 ),
	vec2( -0.382775, 0.276768 ),
	vec2( -0.264969, -0.41893 ),
	vec2( 0.53743, -0.473734 ),
	vec2( 0.199841, 0.786414 ),
	vec2( 0.791975, 0.190902 ),
	vec2( -0.0941841, -0.929389 ),
	vec2( -0.942016, -0.399062 ),
	vec2( -0.915886, 0.457714 ),
	vec2( -0.241888, 0.997065 ),
	vec2( 0.443233, -0.975116 ),
	vec2( -0.815442, -0.879125 ),
	vec2( 0.945586, -0.768907 ),
	vec2( -0.8141, 0.914376 ),
	vec2( 0.974844, 0.756484 )
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
			cascadeCoef = 3.0;
		if(cascadeIndex > 2)
			cascadeCoef = 4.0;

		float distance = depthDifference(projCoords, cascadeIndex);
		float divisor = mix(700.0, 300.0, distance * 10.0) * cascadeCoef;
		
		int nIteration = clamp(int(mix(4, 16, distance * 10.0)), 4, 16);

		divisor = 700.0 * cascadeCoef;
		nIteration = 16 / int(cascadeCoef);

		for(int i = 0; i < nIteration; i++)
		{
			int index = int(float(nIteration)*random(worldPos.xyz, i))%nIteration;
			shadow += 1.0 - textureProj(projCoords + vec4(poissonDisk[index] / divisor, 0.0, 0.0), cascadeIndex);
		}
		shadow /= float(nIteration);
	}	

    outColor = vec4(shadow.rrr, 1.0);
}