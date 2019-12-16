#include "RayTracingPass.h"

void RayTracingPass::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model)
{
	std::vector<VertexBuffer> modelVertexBuffers = model->getVertexBuffers();
	std::vector<GeometryInstance> geometryInstances(modelVertexBuffers.size());
	for (int i(0); i < geometryInstances.size(); ++i)
	{
		geometryInstances[i].vertexBuffer = modelVertexBuffers[i].vertexBuffer;
		geometryInstances[i].vertexOffset = 0;
		geometryInstances[i].vertexSize = sizeof(VertexPBR);
		geometryInstances[i].vertexCount = modelVertexBuffers[i].nbVertices;
		geometryInstances[i].indexBuffer = modelVertexBuffers[i].indexBuffer;
		geometryInstances[i].indexOffset = 0;
		geometryInstances[i].indexCount = modelVertexBuffers[i].nbIndices;
		geometryInstances[i].transform = model->getTransformation();
	}

	m_accelerationStructure.initialize(device, physicalDevice, commandPool, graphicsQueue, geometryInstances);
}

void RayTracingPass::createRaytracingDescriptorSet(VkDevice device, std::vector<VkSampler> textureSamplers)
{
	// Add the bindings to the resources
	// Top-level acceleration structure, usable by both the ray generation and the
	// closest hit (to shoot shadow rays)
	m_rtDSG.AddBinding(0, 1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, VK_SHADER_STAGE_RAYGEN_BIT_NV);
	// Raytracing output
	m_rtDSG.AddBinding(1, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV);
	// Camera information
	m_rtDSG.AddBinding(2, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_NV);
	// Scene data
	// Vertex buffer
	m_rtDSG.AddBinding(3, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
	// Index buffer
	m_rtDSG.AddBinding(4, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
	// Material buffer
	m_rtDSG.AddBinding(5, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
	// Textures
	m_rtDSG.AddBinding(6, static_cast<uint32_t>(textureSamplers.size()), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);

	// Create the descriptor pool and layout
	m_rtDescriptorPool = m_rtDSG.GeneratePool(device);
	m_rtDescriptorSetLayout = m_rtDSG.GenerateLayout(device);

	// Generate the descriptor set
	m_rtDescriptorSet = m_rtDSG.GenerateSet(device, m_rtDescriptorPool, m_rtDescriptorSetLayout);

	// Bind the actual resources into the descriptor set
	// Top-level acceleration structure
	//VkWriteDescriptorSetAccelerationStructureNV descriptorAccelerationStructureInfo;
	//descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
	//descriptorAccelerationStructureInfo.pNext = nullptr;
	//descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
	//descriptorAccelerationStructureInfo.pAccelerationStructures = &m_topLevelAS.structure;

	//m_rtDSG.Bind(m_rtDescriptorSet, 0, { descriptorAccelerationStructureInfo });
}
