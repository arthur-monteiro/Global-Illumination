#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 mvp;
    mat4 lightSpaceMatrices[4];
    vec4 cascadeSplits;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in uint inMaterialID;

layout(location = 0) out vec3 viewPos;
layout(location = 1) out vec4 cascadeSplits;
layout(location = 2) out vec4 posLightSpace[4];

out gl_PerVertex
{
    vec4 gl_Position;
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main()
{
    gl_Position = ubo.mvp * vec4(inPosition, 1.0);
    viewPos = gl_Position.xyz;
    cascadeSplits = ubo.cascadeSplits;

    for(int i = 0; i < 4; i++)
        posLightSpace[i] = biasMat * ubo.lightSpaceMatrices[i] * vec4(inPosition, 1.0);
}  