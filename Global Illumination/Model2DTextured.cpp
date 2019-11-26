#include "Model2DTextured.h"

Model2DTextured::~Model2DTextured()
{
}

int Model2DTextured::addMeshFromVertices(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<Vertex2DTextured> vertices, std::vector<uint32_t> indices)
{
	Mesh<Vertex2DTextured> mesh;
	mesh.loadFromVertices(device, physicalDevice, commandPool, graphicsQueue, std::move(vertices), std::move(indices));

	m_meshes.push_back(mesh);

	return m_meshes.size() - 1;
}

void Model2DTextured::cleanup(VkDevice device)
{
	for (int i(0); i < m_meshes.size(); ++i)
		m_meshes[i].cleanup(device);
}

std::vector<VertexBuffer> Model2DTextured::getVertexBuffers()
{
	std::vector<VertexBuffer> vertexBuffers;

	for (int i(0); i < m_meshes.size(); ++i)
	{
		vertexBuffers.push_back(m_meshes[i].getVertexBuffer());
	}

	return vertexBuffers;
}
