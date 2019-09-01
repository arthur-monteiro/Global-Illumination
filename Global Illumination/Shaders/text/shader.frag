#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObjectColor
{
	vec3 color;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
    if(ubo.color.x < 0)
    {
        outColor = vec4(0.0);
        return;
    }
    outColor = vec4(ubo.color, texture(texSampler, fragTexCoord).r);
}