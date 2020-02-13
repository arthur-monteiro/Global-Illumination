#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 1) uniform sampler2D[] textures;

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
	outAlbedo = texture(textures[inMaterialID * 5], inTexCoord);
	vec3 normal = (texture(textures[inMaterialID * 5 + 1], inTexCoord).rgb * 2.0 - vec3(1.0)) * inTBN;
	normal = normalize(normal);
	outNormal = vec4(normal, 1.0);
	outRoughnessMetalAO = vec4(texture(textures[inMaterialID * 5 + 2], inTexCoord).r, texture(textures[inMaterialID * 5 + 3], inTexCoord).r, texture(textures[inMaterialID * 5 + 4], inTexCoord).r, 1.0);
	outViewPos = vec4(inViewdPos, 1.0);
}
