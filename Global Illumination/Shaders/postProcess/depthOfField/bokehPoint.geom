#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec4 inPosition[];
layout(location = 1) in vec4 inBlurAndColor[];

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoord;

float sizeFactorX = 0.03;
float sizeFactorY = sizeFactorX * (16.0 / 9.0);

void main()
{
    outColor = vec4(inBlurAndColor[0].gba, 1.0);
    outTexCoord = vec2(0.0, 0.0);
    gl_Position = gl_in[0].gl_Position + vec4(-sizeFactorX * inBlurAndColor[0].x, -sizeFactorY * inBlurAndColor[0].x, 0.0, 0.0); 
    EmitVertex();

    outColor = vec4(inBlurAndColor[0].gba, 1.0);
    outTexCoord = vec2(0.0, 1.0);
    gl_Position = gl_in[0].gl_Position + vec4(-sizeFactorX * inBlurAndColor[0].x, sizeFactorY * inBlurAndColor[0].x, 0.0, 0.0); 
    EmitVertex();

    outColor = vec4(inBlurAndColor[0].gba, 1.0);
    outTexCoord = vec2(1.0, 0.0);
    gl_Position = gl_in[0].gl_Position + vec4(sizeFactorX * inBlurAndColor[0].x, -sizeFactorY * inBlurAndColor[0].x, 0.0, 0.0); 
    EmitVertex();

    outColor = vec4(inBlurAndColor[0].gba, 1.0);
    outTexCoord = vec2(1.0, 1.0);
    gl_Position = gl_in[0].gl_Position+ vec4(sizeFactorX * inBlurAndColor[0].x, sizeFactorY * inBlurAndColor[0].x, 0.0, 0.0); 
    EmitVertex();
    
    EndPrimitive();
}