#pragma once

#include "Vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

struct UniformBufferSingleFloat
{
	float floatVal;
};

struct UniformBufferObjectSingleVec
{
	glm::vec4 vecValue;
};

struct UniformBufferObjectVP
{
	glm::mat4 view;
	glm::mat4 proj;
};

struct UniformBufferObjectSingleMat
{
	glm::mat4 matrix;
};

struct UniformBufferObjectArrayMat
{
	std::vector<glm::mat4> matrices;

	VkDeviceSize getSize()
	{
		return matrices.size() * sizeof(glm::mat4);
	}

	void* getData()
	{
		void* r = malloc(getSize());
		memcpy(r, matrices.data(), getSize());

		return r;
	}
};

const int MAX_POINTLIGHTS = 32;
const int MAX_DIRLIGHTS = 1;

struct UniformBufferObjectLights
{
	glm::vec4 camPos = glm::vec4(2.0f);

	std::array<glm::vec4, MAX_POINTLIGHTS> pointLightsPositions;
	std::array<glm::vec4, MAX_POINTLIGHTS> pointLightsColors;
	uint32_t nbPointLights = 0;

	std::array<glm::vec4, MAX_DIRLIGHTS> dirLightsDirections;
	std::array<glm::vec4, MAX_DIRLIGHTS> dirLightsColors;
	uint32_t nbDirLights = 0;
};

struct UniformBufferObjectDirLight
{
	glm::vec4 camPos;

	glm::vec4 dirLight;
	glm::vec4 colorLight;

	float usePCF;
	float ambient;
};

struct UniformBufferObjectDirLightCSM
{
	glm::vec4 camPos;

	glm::vec4 dirLight;
	glm::vec4 colorLight;

	float ambient;
};

struct UniformBufferObjectCSM
{
	std::vector<glm::vec4> cascadeSplits;

	VkDeviceSize getSize()
	{
		return cascadeSplits.size() * sizeof(glm::vec4);
	}

	void* getData()
	{
		void* r = malloc(getSize());
		memcpy(r, cascadeSplits.data(), getSize());

		return r;
	}
};

struct UniformBufferObjectItemQuad
{
	glm::vec4 color;
	glm::mat4 transform;
};

class UboBase {
public:
	virtual ~UboBase() {}
	//virtual std::string Get() = 0;

	VkBuffer getUniformBuffer() { return m_uniformBuffer; }
	VkDeviceMemory getUniformBufferMemory() { return m_uniformBufferMemory; }
	VkDeviceSize getSize() { return m_size; }
	VkShaderStageFlags getAccessibility() { return m_accessibility; }

protected:
	bool m_isInitialized = false;

	VkBuffer m_uniformBuffer;
	VkDeviceMemory m_uniformBufferMemory;

	VkDeviceSize m_size = 0;

	VkShaderStageFlags m_accessibility = VK_SHADER_STAGE_VERTEX_BIT;
};

template <typename T>
class UniformBufferObject : public UboBase
{
public:
	UniformBufferObject() {}
	~UniformBufferObject()
	{
		if (m_isInitialized)
			std::cout << "UBO not destroyed !" << std::endl;
	}

	void load(Vulkan* vk, T data, VkShaderStageFlags accessibility)
	{
		if (m_isInitialized)
			return;

		VkDeviceSize bufferSize = sizeof(T);
		vk->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffer, m_uniformBufferMemory);

		m_size = sizeof(T);
		m_accessibility = accessibility;

		void* pData;
		vkMapMemory(vk->getDevice(), m_uniformBufferMemory, 0, m_size, 0, &pData);
			memcpy(pData, &data, m_size);
		vkUnmapMemory(vk->getDevice(), m_uniformBufferMemory);

		m_isInitialized = true;
	}

	void load(Vulkan* vk, void* data, VkDeviceSize size, VkShaderStageFlags accessibility)
	{
		vk->createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffer, m_uniformBufferMemory);

		m_size = size;
		m_accessibility = accessibility;

		void* pData;
		vkMapMemory(vk->getDevice(), m_uniformBufferMemory, 0, m_size, 0, &pData);
			memcpy(pData, data, m_size);
		vkUnmapMemory(vk->getDevice(), m_uniformBufferMemory);

		free(data);
	}

	void update(Vulkan* vk, T data)
	{
		void* pData;
		vkMapMemory(vk->getDevice(), m_uniformBufferMemory, 0, m_size, 0, &pData);
			memcpy(pData, &data, m_size);
		vkUnmapMemory(vk->getDevice(), m_uniformBufferMemory);
	}

	void update(Vulkan* vk, void* data, VkDeviceSize size)
	{
		void* pData;
		vkMapMemory(vk->getDevice(), m_uniformBufferMemory, 0, m_size, 0, &pData);
			memcpy(pData, data, size);
		vkUnmapMemory(vk->getDevice(), m_uniformBufferMemory);
	}

	void cleanup(VkDevice device)
	{
		vkDestroyBuffer(device, m_uniformBuffer, nullptr);
		vkFreeMemory(device, m_uniformBufferMemory, nullptr);

		m_isInitialized = false;
	}

private:
};