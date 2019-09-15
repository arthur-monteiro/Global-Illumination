#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObjectVP
{
    mat4 view;
    mat4 proj;
} uboVP;


layout(binding = 1) uniform UniformBufferObjectModel
{
    mat4 model;
} uboModel;

#define CASCADES_COUNT 4

layout(binding = 2) uniform UniformBufferObjectLightSpace
{
    mat4 lightSpaces[CASCADES_COUNT];
} uboLightSpace;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 viewPos;
layout(location = 1) out vec4 posLightSpace[CASCADES_COUNT];

out gl_PerVertex
{
    vec4 gl_Position;
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
    gl_Position = uboVP.proj * uboVP.view * uboModel.model * vec4(inPosition, 1.0);
	
	viewPos = gl_Position.xyz;
	for(int i = 0; i < CASCADES_COUNT; i++)
    	posLightSpace[i] = biasMat * uboLightSpace.lightSpaces[i] * vec4(inPosition, 1.0);
} 