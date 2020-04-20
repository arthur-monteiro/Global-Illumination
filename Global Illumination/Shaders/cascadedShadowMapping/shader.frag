#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

#define CASCADES_COUNT 4

layout (binding = 1) uniform sampler shadowMapSampler;
layout (binding = 2) uniform texture2D[] shadowMaps;

layout(location = 0) in vec3 viewPos;
layout(location = 1) in vec4 cascadeSplits;
layout(location = 2) in vec4 posLightSpace[4];
layout(location = 6) in flat ivec4 softShadowOption;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
const float BIAS = 0.0001;

float textureProj(vec4 shadowCoord, uint cascadeIndex)
{
	float closestDepth = texture(sampler2D(shadowMaps[cascadeIndex], shadowMapSampler), shadowCoord.st).r; 
    float currentDepth = shadowCoord.z;

    float shadow = currentDepth - BIAS > closestDepth  ? 1.0 : 0.0;

	return shadow;
}

vec2 poissonDisk[64] = vec2[]( 
	vec2( 0.063326, 0.142369 ),
	vec2( 0.155736, 0.065157 ),
	vec2( 0.170019, -0.040254 ),
	vec2( 0.203528, 0.214331 ),
	vec2( 0.222999, -0.215125 ),
	vec2( -0.098422, -0.295755 ),
	vec2( 0.099094, -0.308023 ),
	vec2( 0.034028, 0.325968 ),
	vec2( 0.039766, -0.3961 ),
	vec2( 0.175817, 0.382366 ),
	vec2( 0.421003, 0.02707 ),
	vec2( -0.44084, 0.137486 ),
	vec2( 0.487472, -0.063082 ),
	vec2( 0.464034, -0.188818 ),
	vec2( 0.470016, 0.217933 ),
	vec2( 0.200476, 0.49443 ),
	vec2( -0.075838, -0.529344 ),
	vec2( 0.385784, -0.393902 ),
	vec2( 0.503098, -0.308878 ),
	vec2( 0.062655, -0.611866 ),
	vec2( -0.467574, -0.405438 ),
	vec2( -0.178564, -0.596057 ),
	vec2( -0.149693, 0.605762 ),
	vec2( 0.50444, 0.372295 ),
	vec2( 0.364483, 0.511704 ),
	vec2( 0.634388, -0.049471 ),
	vec2( 0.643361, 0.164098 ),
	vec2( 0.315226, -0.604297 ),
	vec2( -0.688894, 0.007843 ),
	vec2( -0.620106, -0.328104 ),
	vec2( 0.678884, -0.204688 ),
	vec2( 0.078707, -0.715323 ),
	vec2( -0.667531, 0.32609 ),
	vec2( -0.545396, 0.538133 ),
	vec2( -0.772454, -0.090976 ),
	vec2( 0.001801, 0.780328 ),
	vec2( 0.69396, -0.366253 ),
	vec2( 0.64568, 0.49321 ),
	vec2( 0.566637, 0.605213 ),
	vec2( -0.299417, 0.791925 ),
	vec2( -0.248268, -0.814753 ),
	vec2( -0.817194, -0.271096 ),
	vec2( -0.494552, -0.711051 ),
	vec2( -0.613392, 0.617481 ),
	vec2( -0.146886, -0.859249 ),
	vec2( -0.016205, -0.872921 ),
	vec2( 0.751946, 0.453352 ),
	vec2( -0.69689, -0.549791 ),
	vec2( 0.789239, -0.419965 ),
	vec2( -0.084078, 0.898312 ),
	vec2( 0.145177, -0.898984 ),
	vec2( -0.885922, 0.215369 ),
	vec2( -0.780145, 0.486251 ),
	vec2( 0.488876, -0.783441 ),
	vec2( 0.724479, -0.580798 ),
	vec2( 0.612476, 0.705252 ),
	vec2( 0.391522, 0.849605 ),
	vec2( 0.354411, -0.88757 ),
	vec2( -0.371868, 0.882138 ),
	vec2( -0.578845, -0.768792 ),
	vec2( -0.651784, 0.717887 ),
	vec2( -0.705374, -0.668203 ),
	vec2( 0.034211, 0.97998 ),
	vec2( 0.97705, -0.108615 )
);

float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

// https://www.gamedev.net/tutorials/programming/graphics/contact-hardening-soft-shadows-made-fast-r4906/
vec2 vogelDiskSample(int sampleIndex, int samplesCount, float phi)
{
  float GoldenAngle = 2.4;

  float r = sqrt(sampleIndex + 0.5) / sqrt(samplesCount);
  float theta = sampleIndex * GoldenAngle + phi;

  float sine = sin(theta);
  float cosine = cos(theta);
  
  return vec2(r * cosine, r * sine);
}

float interleavedGradientNoise(vec2 position_screen)
{
  vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
  return fract(magic.z * fract(dot(position_screen, magic.xy)));
}

void main() 
{
	uint cascadeIndex = 0;
	for(uint i = 0; i < CASCADES_COUNT; ++i) 
	{
		if(viewPos.z <= cascadeSplits[i])
		{	
			cascadeIndex = i;
			break;
		}
	}

	float texelSizeX = 1.0 / 2048.0;
	if(cascadeIndex > 0)
		texelSizeX = 1.0 / 2048.0;
	if(cascadeIndex > 2)
		texelSizeX = 1.0 / 1024.0;

	vec4 projCoords = posLightSpace[cascadeIndex] / posLightSpace[cascadeIndex].w; 
	float shadow = 0.0;
	float penombra = 0.0;
	if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
	{
		outColor = vec4(0.0, 0.0, 1.0, 1.0);
		return;
	}
	else 
	{
		if(softShadowOption.x == 0.0)
			shadow += 1.0 - textureProj(projCoords, cascadeIndex);
		else if(softShadowOption.x > 0.5 && softShadowOption.x < 1.5) // Poisson
		{
			float divisor = softShadowOption.z;
			float nIteration = softShadowOption.y;

			float step = 64.0 / nIteration; 

			for(int i = 0; i < int(nIteration); i++)
			{
				vec2 offset = poissonDisk[int(i * step)].xy;
				shadow += 1.0 - textureProj(projCoords + vec4(offset / divisor, 0.0, 0.0), cascadeIndex);
			}
			shadow /= nIteration;
		}
		else if(softShadowOption.x > 1.5 && softShadowOption.x < 2.5) // Poisson stratified
		{
			float divisor = softShadowOption.z;
			float nIteration = softShadowOption.y;

			float step = 64.0 / nIteration; 

			vec2 randomValue = vec2(random(gl_FragCoord.xyz, 0), random(gl_FragCoord.xyz, 1));
			randomValue *= 2.0;
			randomValue -= vec2(1.0);

			for(int i = 0; i < int(nIteration); i++)
			{
				vec2 offset = vec2(randomValue.x * poissonDisk[int(i * step)].x - randomValue.y * poissonDisk[int(i * step)].y,
					randomValue.y * poissonDisk[int(i * step)].x + randomValue.x * poissonDisk[int(i * step)].y);
				shadow += 1.0 - textureProj(projCoords + vec4(offset / divisor, 0.0, 0.0), cascadeIndex);
			}
			shadow /= nIteration;
		}		
		else if(softShadowOption.x > 2.5 && softShadowOption.x < 3.5) // PCF
		{
			int nIteration = int(softShadowOption.y);

			for(int i = -nIteration / 2; i < nIteration / 2; i++)
			{
				for(int j = -nIteration / 2; j < nIteration / 2; ++j)
					shadow += 1.0 - textureProj(projCoords + vec4(texelSizeX * float(i), texelSizeX * float(j), 0.0, 0.0), cascadeIndex);
			}
			shadow /= nIteration * nIteration;
		}
		else if(softShadowOption.x > 3.5 && softShadowOption.x < 4.5) // Vogel
		{
			int nIteration = int(softShadowOption.y);
			float divisor = softShadowOption.z;

			for(int i = 0; i < nIteration; ++i)
				shadow += 1.0 - textureProj(projCoords + vec4(vogelDiskSample(i, nIteration, 0.0) / divisor, 0.0, 0.0), cascadeIndex);

			shadow /= nIteration;
		}
		else if(softShadowOption.x > 4.5 && softShadowOption.x < 5.5) // Vogel Noise
		{
			int nIteration = int(softShadowOption.y);
			float divisor = softShadowOption.z;

			for(int i = 0; i < nIteration; ++i)
				shadow += 1.0 - textureProj(projCoords + vec4(vogelDiskSample(i, nIteration, interleavedGradientNoise(gl_FragCoord.xy)) / divisor, 0.0, 0.0), cascadeIndex);

			shadow /= nIteration;
		}
		else if(softShadowOption.x > 5.5 && softShadowOption.x < 6.5)
		{
			int nIteration = int(softShadowOption.y);
			float divisor = softShadowOption.z;

			float avgBlockersDepth = 0.0f;
			float blockerCount = 0.0f;
			for(int i = 0; i < nIteration; ++i)
			{
				vec4 samplingCoords = projCoords + vec4(vogelDiskSample(i, nIteration, interleavedGradientNoise(gl_FragCoord.xy)) / divisor, 0.0, 0.0);
				if(textureProj(samplingCoords, cascadeIndex) == 1.0)
				{
					blockerCount += 1.0f;
					avgBlockersDepth += texture(sampler2D(shadowMaps[cascadeIndex], shadowMapSampler), samplingCoords.st).r;
				}
			}

			float fPenumbra = 0.0;
			if(blockerCount > 0.0)
			{
				avgBlockersDepth /= blockerCount;

				float tPenumbra = (projCoords.z - avgBlockersDepth);
				fPenumbra = clamp(tPenumbra * 100.0, 0.4, 1.0);
				fPenumbra *= fPenumbra;
			}
			else
				fPenumbra = 0.0;

			for(int i = 0; i < nIteration; ++i)
				shadow += 1.0 - textureProj(projCoords + 
					vec4((vogelDiskSample(i, nIteration, interleavedGradientNoise(gl_FragCoord.xy)) / divisor) * fPenumbra, 0.0, 0.0), cascadeIndex);

			shadow /= nIteration;
		}
	}	

    outColor = vec4(shadow, viewPos.z / 100.0, 0.0, 1.0);
}