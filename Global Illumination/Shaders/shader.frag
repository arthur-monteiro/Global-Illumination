#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 2) uniform sampler2D texNormal;
layout(binding = 3) uniform sampler2D texRoughness;
layout(binding = 4) uniform sampler2D texMetal;
layout(binding = 5) uniform sampler2D texAO;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in mat3 inTBN;

layout(location = 0) out vec4 outAlbedo;

void main() 
{
	outAlbedo = texture(texAlbedo, inTexCoord);
}
