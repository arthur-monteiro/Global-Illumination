#include "Mesh2D.h"

void Mesh2D::loadFromVertices(VkDevice device, std::vector<Vertex2D> vertices, std::vector<uint32_t> indices)
{
    m_vertices = vertices;
    m_indices = indices;
}

void Mesh2D::cleanup(VkDevice device)
{

}