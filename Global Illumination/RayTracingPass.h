#pragma once

#include "VulkanHelper.h"
#include "ModelPBR.h"
#include "AccelerationStructure.h"
#include "vulkannv/nv_helpers_vk/DescriptorSetGenerator.h"
#include "UniformBufferObject.h"

class RayTracingPass
{
public:
	RayTracingPass() = default;
	~RayTracingPass();

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model, glm::mat4 mvp);

	void cleanup(VkDevice device);

private:
	void createRaytracingDescriptorSet(VkDevice device, std::vector<Texture*> textures, VkBuffer vertexBuffer, VkBuffer indexBuffer);

private:
	struct MVP_UBO
	{
		glm::mat4 mvp;
		glm::mat4 model;
	};

	UniformBufferObject m_uboMVP;

	AccelerationStructure m_accelerationStructure;

	nv_helpers_vk::DescriptorSetGenerator m_rtDSG;
	VkDescriptorPool m_rtDescriptorPool;
	VkDescriptorSetLayout m_rtDescriptorSetLayout;
	VkDescriptorSet m_rtDescriptorSet;

	Image m_imageTarget;
};

