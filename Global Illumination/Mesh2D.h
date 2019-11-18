#pragma once

#include "Mesh.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

class Mesh2D : MeshBase // Replace with a template ?
{
public:
    void loadFromVertices(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<Vertex2D> vertices, std::vector<uint32_t> indices);

    void cleanup(VkDevice device);

private:
    std::vector<Vertex2D> m_vertices;
};