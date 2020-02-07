#include "RayTracingPass.h"

RayTracingPass::~RayTracingPass()
{
}

void RayTracingPass::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkQueue graphicsQueue, std::vector<GeometryInstance> geometryInstances)
{
	m_accelerationStructure.initialize(device, physicalDevice, commandPool, graphicsQueue, geometryInstances);
}

