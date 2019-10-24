#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform UniformBufferObjectLights
{
	vec4 camPos;
	
	vec4 dirDirLight;
	vec4 colorDirLight;

	float usePCF;
	float ambient;
} uboLights;

layout(binding = 2) uniform UniformBufferObjectRadiosityProbes
{
	vec4 radiosity[1000];
} uboRadiosityProbes;

layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;

layout(location = 3) in flat int id;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(uboRadiosityProbes.radiosity[id].rrr, 1.0);
}