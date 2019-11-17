#pragma once

#include "Mesh.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

struct Vertex2D
{
    glm::vec2 pos;

    static VkVertexInputBindingDescription getBindingDescription(uint32_t binding)
    {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = binding;
        bindingDescription.stride = sizeof(Vertex2D);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding)
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

        attributeDescriptions[0].binding = binding;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex2D, pos);

        return attributeDescriptions;
    }

    bool operator==(const Vertex2D& other) const
    {
        return pos == other.pos;
    }
};


class Mesh2D : MeshBase
{
public:
    void loadFromVertices(VkDevice device, std::vector<Vertex2D> vertices, std::vector<uint32_t> indices);

    void cleanup(VkDevice device);

private:
    std::vector<Vertex2D> m_vertices;
};