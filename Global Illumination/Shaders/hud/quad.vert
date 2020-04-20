#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in uint inID;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outPosition;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 transform[64];
	vec4 color[64];
} ubo;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = ubo.transform[inID] * vec4(inPosition, 0.0, 1.0);
    
    outPosition = gl_Position.xy;
    outColor = ubo.color[inID];
}  