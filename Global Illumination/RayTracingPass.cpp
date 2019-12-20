#include "RayTracingPass.h"

RayTracingPass::~RayTracingPass()
{
}

void RayTracingPass::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model, glm::mat4 mvp)
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

	MVP_UBO uboData;
	uboData.model = model->getTransformation();
	uboData.mvp = mvp;
	m_uboMVP.initialize(device, physicalDevice, &uboData, sizeof(MVP_UBO));

	m_imageTarget.create(device, physicalDevice, { 1280, 720 }, VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_imageTarget.setImageLayout(device, commandPool, graphicsQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	std::vector<VertexBuffer> vertexBuffers = model->getVertexBuffers();
	createRaytracingDescriptorSet(device, model->getTextures(0), vertexBuffers[0].vertexBuffer, vertexBuffers[0].indexBuffer);
}

void RayTracingPass::cleanup(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, m_rtDescriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device, m_rtDescriptorPool, nullptr);
	m_accelerationStructure.cleanup(device);
}

void RayTracingPass::createRaytracingDescriptorSet(VkDevice device, std::vector<Texture*> textures, VkBuffer vertexBuffer, VkBuffer indexBuffer)
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
	// Textures
	m_rtDSG.AddBinding(5, static_cast<uint32_t>(textures.size()), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);

	// Create the descriptor pool and layout
	m_rtDescriptorPool = m_rtDSG.GeneratePool(device);
	m_rtDescriptorSetLayout = m_rtDSG.GenerateLayout(device);

	// Generate the descriptor set
	m_rtDescriptorSet = m_rtDSG.GenerateSet(device, m_rtDescriptorPool, m_rtDescriptorSetLayout);

	// Bind the actual resources into the descriptor set
	// Top-level acceleration structure
	VkWriteDescriptorSetAccelerationStructureNV descriptorAccelerationStructureInfo;
	descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
	descriptorAccelerationStructureInfo.pNext = nullptr;
	descriptorAccelerationStructureInfo.accelerationStructureCount = 1;

	VkAccelerationStructureNV topLevelAS = m_accelerationStructure.getToLevelAccelerationStructure();
	descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAS;

	m_rtDSG.Bind(m_rtDescriptorSet, 0, { descriptorAccelerationStructureInfo });

	// Camera matrices
	VkDescriptorBufferInfo camInfo = {};
	camInfo.buffer = m_uboMVP.getUniformBuffer();
	camInfo.offset = 0;
	camInfo.range = sizeof(MVP_UBO);

	m_rtDSG.Bind(m_rtDescriptorSet, 2, { camInfo });

	// Vertex buffer
	VkDescriptorBufferInfo vertexInfo = {};
	vertexInfo.buffer = vertexBuffer;
	vertexInfo.offset = 0;
	vertexInfo.range = VK_WHOLE_SIZE;

	m_rtDSG.Bind(m_rtDescriptorSet, 3, { vertexInfo });

	// Index buffer
	VkDescriptorBufferInfo indexInfo = {};
	indexInfo.buffer = indexBuffer;
	indexInfo.offset = 0;
	indexInfo.range = VK_WHOLE_SIZE;

	m_rtDSG.Bind(m_rtDescriptorSet, 4, { indexInfo });

	// Textures
	std::vector<VkDescriptorImageInfo> imageInfos;
	for (size_t i = 0; i < textures.size(); ++i)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textures[i]->getImageView();
		imageInfo.sampler = textures[i]->getSampler();
		imageInfos.push_back(imageInfo);
	}
	if (!textures.empty())
	{
		m_rtDSG.Bind(m_rtDescriptorSet, 5, imageInfos);
	}

	// Output buffer
	VkDescriptorImageInfo descriptorOutputImageInfo;
	descriptorOutputImageInfo.sampler = nullptr;
	descriptorOutputImageInfo.imageView = m_imageTarget.getImageView();
	descriptorOutputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	m_rtDSG.Bind(m_rtDescriptorSet, 1, { descriptorOutputImageInfo });

	// Copy the bound resource handles into the descriptor set
	m_rtDSG.UpdateSetContents(device, m_rtDescriptorSet);
}
