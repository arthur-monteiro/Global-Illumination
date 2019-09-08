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

	void load(Vulkan* vk, T data, VkShaderStageFlags accessibility)
	{
		VkDeviceSize bufferSize = sizeof(T);
		vk->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffer, m_uniformBufferMemory);

		m_size = sizeof(T);
		m_accessibility = accessibility;

		void* pData;
		vkMapMemory(vk->getDevice(), m_uniformBufferMemory, 0, m_size, 0, &pData);
			memcpy(pData, &data, m_size);
		vkUnmapMemory(vk->getDevice(), m_uniformBufferMemory);
	}

	void update(Vulkan* vk, T data)
	{
		void* pData;
		vkMapMemory(vk->getDevice(), m_uniformBufferMemory, 0, m_size, 0, &pData);
			memcpy(pData, &data, m_size);
		vkUnmapMemory(vk->getDevice(), m_uniformBufferMemory);
	}

	void cleanup(VkDevice device)
	{
		vkDestroyBuffer(device, m_uniformBuffer, nullptr);
		vkFreeMemory(device, m_uniformBufferMemory, nullptr);
	}

private:
};