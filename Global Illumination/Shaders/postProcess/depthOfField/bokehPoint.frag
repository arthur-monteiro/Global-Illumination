#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inTexCoord;

layout (binding = 0) uniform sampler2D inBokehShape;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(inColor.rgb * 0.05 * texture(inBokehShape, inTexCoord).r, 1.0);
}