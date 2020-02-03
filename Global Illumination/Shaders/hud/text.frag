#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 0) uniform UniformBufferObjectColor
{
	vec3 color[32];
} ubo;

layout (binding = 1) uniform sampler2D[] textures;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) flat in uvec3 inIDs;

layout(location = 0) out vec4 outColor; 	

void main() 
{
	if(ubo.color[inIDs[1]].r < 0)
    {
        outColor = vec4(0.0);
        return;
    }
    outColor = vec4(ubo.color[inIDs[1]] * texture(textures[inIDs[0]], inTexCoord).r, texture(textures[inIDs[0]], inTexCoord).r);
}