#pragma once

#include <unordered_map>

#include "Vulkan.h"
#include "Pipeline.h"

struct Image
{
	VkImage image;
	VkDeviceMemory  imageMemory;
	VkImageView imageView;
};

class MeshBase
{
public:
	virtual ~MeshBase() 
	{
		if (!m_isDestroyed)
			std::cout << "Mesh not destroyed !" << std::endl;
	}

	void createTextureSampler(Vulkan* vk, VkSamplerAddressMode addressMode);
	int createTexture(Vulkan* vk, uint32_t height, uint32_t width, int mipLevels, int nLayers);
	void loadTextureFromFile(Vulkan* vk, std::vector<std::string> path);
	void loadTextureFromImages(Vulkan* vk, std::vector<VkImage> images, uint32_t height, uint32_t width);
	void loadCubemapFromFile(Vulkan* vk, std::vector < std::string > path);
	void loadCubemapFromImages(Vulkan* vk, std::array<VkImage, 6> images, uint32_t height, uint32_t width);
	void loadCubemapFromImages(Vulkan* vk, std::array<VkImage, 6> images, uint32_t height, uint32_t width, int imageID, int mipLevel);
	void loadHDRTexture(Vulkan* vk, std::vector < std::string > path);

	void addImageView(VkImageView imageView);
	void clearImages(VkDevice device);

	std::vector<VkImageView> getImageView()
	{
		std::vector<VkImageView> r;
		for (int i(0); i < m_images.size(); ++i)
			r.push_back(m_images[i].imageView);

		return r;
	}
	VkImageView getImageView(int index) { return m_images[index].imageView; }
	VkSampler getSampler() { return m_textureSampler; }
	VkBuffer getIndexBuffer() { return m_indexBuffer; }
	uint32_t getNumIndices() { return static_cast<uint32_t>(m_indices.size()); }
	VkBuffer getVertexBuffer() { return m_vertexBuffer; }

	void setImageView(int index, VkImageView imageView) { m_images[index].imageView = imageView; }

protected:
	void createIndexBuffer(Vulkan* vk);

	void createTextureImage(Vulkan* vk, std::string path);
	void createTextureImageView(Vulkan* vk, VkFormat format);

protected:
	bool m_isDestroyed = false;

	std::vector<uint32_t> m_indices;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;

	uint32_t m_mipLevels;
	std::vector<Image> m_images;
	VkSampler m_textureSampler = NULL;
};

class MeshPBR : public MeshBase
{
public:
	void loadObj(Vulkan * vk, std::string path, glm::vec3 forceNormal = glm::vec3(-1.0f));

	void restoreTransformations() { m_modelMatrix = glm::mat4(1.0); }
	void rotate(float angle, glm::vec3 dir);
	void scale(glm::vec3 scale);
	void translate(glm::vec3 translation);

	void cleanup(VkDevice device);

private:
	void createVertexBuffer(Vulkan* vk);

public:
	glm::mat4x4 getModelMatrix() { return m_modelMatrix; }

private:
	std::vector<VertexPBR> m_vertices;

	glm::mat4x4 m_modelMatrix;
};

class Mesh2D : public MeshBase
{
public:
	void loadVertices(Vulkan* vk, std::vector<VertexQuad> vertices, std::vector<uint32_t> indices);

	void cleanup(VkDevice device);

private:
	void createVertexBuffer(Vulkan* vk);

private:
	std::vector<VertexQuad> m_vertices;
};