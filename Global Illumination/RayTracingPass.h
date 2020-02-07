#pragma once

#include "VulkanHelper.h"
#include "AccelerationStructure.h"
#include "vulkannv/nv_helpers_vk/DescriptorSetGenerator.h"
#include "UniformBufferObject.h"
#include "RayTracingPipeline.h"
#include "vulkannv/nv_helpers_vk/ShaderBindingTableGenerator.h"
#include "Command.h"

class RayTracingPass
{
public:
	RayTracingPass() = default;
	~RayTracingPass();

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<GeometryInstance> geometryInstances);

	VkAccelerationStructureNV getToLevelAccelerationStructure() { return m_accelerationStructure.getToLevelAccelerationStructure(); }

private:
	AccelerationStructure m_accelerationStructure;
};

