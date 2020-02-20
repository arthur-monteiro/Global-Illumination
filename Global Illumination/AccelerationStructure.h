// https://developer.nvidia.com/rtx/raytracing/vkray

#pragma once

#include "VulkanHelper.h"
#include "Model.h"
#include "vulkannv/nv_helpers_vk/BottomLevelASGenerator.h"
#include "vulkannv/nv_helpers_vk/TopLevelASGenerator.h"

struct GeometryInstance
{
	VkBuffer vertexBuffer;
	uint32_t vertexCount;
	VkDeviceSize vertexOffset;
	VkDeviceSize vertexSize;
	VkBuffer indexBuffer;
	uint32_t indexCount;
	VkDeviceSize indexOffset;
	glm::mat4x4 transform;
};

struct AccelerationStructureData
{
	VkBuffer scratchBuffer = VK_NULL_HANDLE;
	VkDeviceMemory scratchMem = VK_NULL_HANDLE;
	VkBuffer resultBuffer = VK_NULL_HANDLE;
	VkDeviceMemory resultMem = VK_NULL_HANDLE;
	VkBuffer instancesBuffer = VK_NULL_HANDLE;
	VkDeviceMemory instancesMem = VK_NULL_HANDLE;
	VkAccelerationStructureNV structure = VK_NULL_HANDLE;
};

class AccelerationStructure
{
public:
	AccelerationStructure() = default;
	~AccelerationStructure();

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<GeometryInstance> geometryInstances);
	void cleanup(VkDevice device);

	VkAccelerationStructureNV getToLevelAccelerationStructure() { return m_topLevelAS.structure; }
		
private:
	AccelerationStructureData createBottomLevelAS(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandBuffer commandBuffer, std::vector<GeometryInstance> geometryInstances);
	void createTopLevelAS(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandBuffer commandBuffer, const std::vector<std::pair<VkAccelerationStructureNV, glm::mat4x4>>& instances);
	void createAccelerationStructures(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<GeometryInstance> geometryInstances);
	void destroyAccelerationStructure(VkDevice device, const AccelerationStructureData& as);


private:
	nv_helpers_vk::TopLevelASGenerator m_topLevelASGenerator;
	AccelerationStructureData m_topLevelAS;
	std::vector<AccelerationStructureData> m_bottomLevelAS;
};

