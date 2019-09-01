#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObjectItem
{
    vec3 color;
    mat4 transform;
} ubo;

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec3 outColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = ubo.transform * vec4(inPosition, 0.0, 1.0);

    outColor = ubo.color;
}  