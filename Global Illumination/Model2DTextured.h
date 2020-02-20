#pragma once

#include "Model.h"
#include "Mesh.h"

class Model2DTextured : public Model
{
public:
	Model2DTextured() = default;
	~Model2DTextured();

	int addMeshFromVertices(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<Vertex2DTextured> vertices, std::vector<uint32_t> indices);

	void cleanup(VkDevice device);

	std::vector<VertexBuffer> getVertexBuffers();

private:
	std::vector<Mesh<Vertex2DTextured>> m_meshes;
};

