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

layout(location = 0) out vec4 outWorldPos;
layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outRoughnessMetalAO;

void main() 
{
	outWorldPos = vec4(inWorldPos, 1.0);

	outAlbedo = texture(texAlbedo, inTexCoord);
	vec3 normal = (texture(texNormal, inTexCoord).rgb * 2.0 - vec3(1.0)) * inTBN;
	normal = normalize(normal); 
	outNormal = vec4(normal, 1.0);
	outRoughnessMetalAO = vec4(texture(texRoughness, inTexCoord).r, texture(texMetal, inTexCoord).r, texture(texAO, inTexCoord).r, 1.0);
}