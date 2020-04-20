#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout (binding = 1) uniform sampler textureSampler;
layout (binding = 2) uniform texture2D[] textures;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inColor;
layout(location = 2) flat in uvec3 inIDs;
layout(location = 3) in vec2 inPosition;

layout(location = 0) out vec4 outColor; 	

void main() 
{
	if(inColor.r < 0 || inPosition.y < -0.7 || inPosition.y > 0.8)
    {
        discard;
    }
    outColor = vec4(inColor/* * texture(textures[inIDs[0]], inTexCoord).r*/, texture(sampler2D(textures[inIDs[0]], textureSampler), inTexCoord).r);
}