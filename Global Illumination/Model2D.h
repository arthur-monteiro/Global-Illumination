#pragma once

#include "Model.h"
#include "Mesh.h"

class Model2D : public Model
{
public:
	Model2D() = default;
	~Model2D();

	int addMeshFromVertices(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<Vertex2D> vertices, std::vector<uint32_t> indices);

private:
	std::vector<Mesh<Vertex2D>> m_meshes;
};

