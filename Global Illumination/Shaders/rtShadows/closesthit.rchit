#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInNV vec3 hitValue;
layout(location = 2) rayPayloadNV bool isShadowed;
hitAttributeNV vec3 attribs;

layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;
layout(binding = 3, set = 0) buffer Vertices { vec4 v[]; } vertices;
layout(binding = 4, set = 0) buffer Indices { uint i[]; } indices;

layout(binding = 5, set = 0) uniform sampler2D[] textureSamplers;

uint vertexSize = 3;

struct Vertex
{
  vec3 pos;
  vec3 norm;
  vec3 tangent;
  vec2 texCoord;
  uint matIndex;
};


Vertex unpackVertex(uint index)
{
  Vertex v;

  vec4 d0 = vertices.v[vertexSize * index + 0];
  vec4 d1 = vertices.v[vertexSize * index + 1];
  vec4 d2 = vertices.v[vertexSize * index + 2];

  v.pos = d0.xyz;
  v.norm = vec3(d0.w, d1.x, d1.y);
  v.tangent = vec3(d1.z, d1.w, d2.x);
  v.texCoord = vec2(d2.y, d2.z);
  v.matIndex = floatBitsToInt(d2.w);

  return v;
}

void main()
{
  ivec3 ind = ivec3(indices.i[3 * gl_PrimitiveID], indices.i[3 * gl_PrimitiveID + 1],
                    indices.i[3 * gl_PrimitiveID + 2]);

  Vertex v0 = unpackVertex(ind.x);
  Vertex v1 = unpackVertex(ind.y);
  Vertex v2 = unpackVertex(ind.z);

  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
  vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;

  vec4 outAlbedo = texture(textureSamplers[v0.matIndex * 5], texCoord);

  float tmin = 0.001;
  float tmax = 100.0;
  vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;
  isShadowed = true;

  vec3 lightVector = vec3(-1.5, 5.0, 1.0);
  traceNV(topLevelAS, 
          gl_RayFlagsTerminateOnFirstHitNV|gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 
          0xFF, 1 /* sbtRecordOffset */, 0 /* sbtRecordStride */,
          1 /* missIndex */, origin, tmin, lightVector, tmax, 2 /*payload location*/);

  if (!isShadowed)
    hitValue = vec3(1.0);
  else 
    hitValue = vec3(0.0);
}
