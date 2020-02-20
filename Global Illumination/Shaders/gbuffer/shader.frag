#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 1) uniform sampler textureSampler;
layout(binding = 2) uniform texture2D[] textures;

layout(location = 0) in vec3 inViewdPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) flat in uint inMaterialID;
layout(location = 3) in mat3 inTBN;

layout(location = 0) out vec4 outViewPos;
layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outRoughnessMetalAO;

void main() 
{
	outAlbedo = texture(sampler2D(textures[inMaterialID * 5], textureSampler), inTexCoord);
	vec3 normal = (texture(sampler2D(textures[inMaterialID * 5 + 1], textureSampler), inTexCoord).rgb * 2.0 - vec3(1.0)) * inTBN;
	normal = normalize(normal);
	outNormal = vec4(normal, outAlbedo.a);
	outRoughnessMetalAO = vec4(texture(sampler2D(textures[inMaterialID * 5 + 2], textureSampler), inTexCoord).r, 
		texture(sampler2D(textures[inMaterialID * 5 + 3], textureSampler), inTexCoord).r,
		texture(sampler2D(textures[inMaterialID * 5 + 4], textureSampler), inTexCoord).r, outAlbedo.a);
	outViewPos = vec4(inViewdPos, outAlbedo.a);
}
