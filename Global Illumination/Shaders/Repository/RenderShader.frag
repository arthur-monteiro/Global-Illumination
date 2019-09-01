#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 2) uniform sampler2D texNormal;
layout(binding = 3) uniform sampler2D texRoughness;
layout(binding = 4) uniform sampler2D texMetal;
layout(binding = 5) uniform sampler2D texAO;

layout(binding = 6) uniform UniformBufferObjectLights
{
    vec3 posPointLights[32];
	vec3 colorPointLights[32];
	int nbPointLights;
	
	vec3 dirDirLights[1];
	vec3 colorDirLights[1];
	int nbDirLights;
} uboLights;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = /*texture(texNormal, fragTexCoord)*/ vec4(ubo.posPointLights[0], 1.0);
}