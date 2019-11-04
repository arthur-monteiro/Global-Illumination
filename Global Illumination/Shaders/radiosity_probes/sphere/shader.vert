#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObjectVP
{
    mat4 view;
    mat4 proj;
} uboVP;

// Per vertex
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

// Per instance
layout(location = 4) in mat4 model;
layout(location = 8) in int id;
// Out

layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 normal;

layout(location = 3) out int outId;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() 
{
    gl_Position = uboVP.proj * uboVP.view * model * vec4(inPosition, 1.0);
	
	mat3 usedModelMatrix = transpose(inverse(mat3(model)));
    normal = usedModelMatrix * inNormal;
	
	worldPos = vec3(model * vec4(inPosition, 1.0));
    fragTexCoord = inTexCoord;

    outId = id;
} 