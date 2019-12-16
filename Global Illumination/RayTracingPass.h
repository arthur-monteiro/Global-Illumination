#pragma once

#include "VulkanHelper.h"
#include "ModelPBR.h"
#include "AccelerationStructure.h"
#include "DescriptorSetGenerator.h"

class RayTracingPass
{
public:
	RayTracingPass() = default;
	~RayTracingPass();

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model);

private:
	void createRaytracingDescriptorSet(VkDevice device, std::vector<VkSampler> textureSamplers);

private:
	AccelerationStructure m_accelerationStructure;

	nv_helpers_vk::DescriptorSetGenerator m_rtDSG;
	VkDescriptorPool m_rtDescriptorPool;
	VkDescriptorSetLayout m_rtDescriptorSetLayout;
	VkDescriptorSet m_rtDescriptorSet;
};

