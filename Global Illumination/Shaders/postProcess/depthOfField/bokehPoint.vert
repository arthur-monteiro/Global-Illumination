#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inBlurAndColor;

layout(location = 0) out vec4 outPos;
layout(location = 1) out vec4 outBlurAndColor;

void main()
{
    outPos = inPosition;
    outBlurAndColor = inBlurAndColor;

    gl_Position = vec4(inPosition.xyz, 1.0);
}