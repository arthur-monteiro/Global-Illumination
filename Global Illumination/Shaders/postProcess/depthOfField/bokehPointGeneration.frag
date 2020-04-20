#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(std430, binding = 0) buffer CommandDraw {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
};

struct BokehPoint
{
  vec4 pos;
  vec4 blurColor;
};

layout(std430, binding = 1) writeonly restrict buffer SSBO {
	BokehPoint bokehPoints[];
};

layout (binding = 2) uniform sampler2D inColor;
layout (binding = 3) uniform sampler2D inDepthBlur;

layout(binding = 4, std140) uniform readonly UniformBufferObjectParams
{
    float brightnessThreshold;
    float blurThreshold;
} uboParams;

void addVertex(vec4 pos, vec4 blurColor)
{
    uint id = atomicAdd(indexCount, 1);
    uint offset = id;

    bokehPoints[offset].pos = pos;
    bokehPoints[offset].blurColor = blurColor;
}

void main() 
{
    vec2 centerCoord = inTexCoord;

	// Start with center sample color
	vec3 centerColor = texture(inColor, inTexCoord).rgb;
	vec3 colorSum = centerColor;
	float totalContribution = 1.0;

    // Sample the depth and blur at the center
	vec2 centerDepthBlur = texture(inDepthBlur, inTexCoord).rg;
	float centerDepth = centerDepthBlur.x;
	float centerBlur = centerDepthBlur.y;

	ivec2 inputTextureSize = textureSize(inColor, 0).xy;
	vec2 texelSize = vec2(1.0) / vec2(inputTextureSize);
 
	uint numSample = 0;
    vec3 averageColor = vec3(0.0);
	float maxValue = 0.0;
	for(int i = -2; i < 2; ++i)
	{
		for(int j = -2; j < 2; ++j)
		{
			vec2 sampleCoord = centerCoord + vec2(i, j) * texelSize;
			vec3 sampleColor = texture(inColor, sampleCoord).rgb;
			if(i != 0 && j != 0 && dot(sampleColor, vec3(1.0)) > maxValue)
				maxValue = dot(sampleColor, vec3(1.0));

			averageColor += sampleColor;
			numSample++;
		}
	}
	averageColor /= float(numSample);

    // Calculate the difference between the current texel and the average
	float averageBrightness = dot(averageColor, vec3(1.0));
	float centerBrightness = dot(centerColor, vec3(1.0));
	float brightnessDiff = max(centerBrightness - averageBrightness, 0.0f);

    if(brightnessDiff >= uboParams.brightnessThreshold && centerBlur > uboParams.blurThreshold)
    {
        addVertex(vec4(inPosition.xy, centerDepth / 32.0, 1.0), vec4(centerBlur, centerColor.x, centerColor.y, centerColor.z));
        centerColor = vec3(0.0);
    }

	outColor = vec4(centerColor, 1.0);
}