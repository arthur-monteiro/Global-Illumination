#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uvec3 inIDs;

layout(location = 0) out vec2 outTexCoords;
layout(location = 1) out uvec3 outIDs;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = vec4(inPosition, 0.0, 1.0);

    outTexCoords = inTexCoord;
    outIDs = inIDs;
}  