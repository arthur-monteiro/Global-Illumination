#include "RayTracingShadows.h"

RayTracingShadows::~RayTracingShadows()
{
}

void RayTracingShadows::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model, glm::mat4 mvp, VkExtent2D extentOutput)
{
	if (m_rtDSG)
		return;
	
	m_rtDSG = std::make_unique<nv_helpers_vk::DescriptorSetGenerator>();
	
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

	InvMvpUBO uboData;
	//uboData.model = model->getTransformation();
	//uboData.mvp = mvp;
	m_uboInvMVP.initialize(device, physicalDevice, &uboData, sizeof(InvMvpUBO));

	m_uboParamsData.sampleCount = 1;
	m_uboParams.initialize(device, physicalDevice, &m_uboParamsData, sizeof(m_uboParamsData));

	m_textureTarget.create(device, physicalDevice, extentOutput, VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_textureTarget.setImageLayout(device, commandPool, graphicsQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_SHADER_WRITE_BIT);

	std::vector<VertexBuffer> vertexBuffers = model->getVertexBuffers();
	createRaytracingDescriptorSet(device, model->getImages(0), model->getSampler(), vertexBuffers[0].vertexBuffer, vertexBuffers[0].indexBuffer);

	m_pipeline.initialize(device, m_rtDescriptorSetLayout, "Shaders/rtShadows/rgen.spv", "Shaders/rtShadows/rmiss.spv", "Shaders/rtShadows/rchit.spv");
	createShaderBindingTable(device, physicalDevice);

	m_command.allocateCommandBuffers(device, commandPool, 1);

	/* Fill command buffer */
	fillCommandBuffer(extentOutput);
}

void RayTracingShadows::submit(VkDevice device, VkQueue queue, std::vector<Semaphore*> waitSemaphores,
	VkSemaphore signalSemaphore, glm::mat4 viewInverse, glm::mat4 projInverse)
{
	InvMvpUBO uboData;
	uboData.viewInverse = viewInverse;
	uboData.projInverse = projInverse;
	m_uboInvMVP.updateData(device, &uboData);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore signalSemaphores[] = { signalSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkCommandBuffer commandBuffer = m_command.getCommandBuffer(0);
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.commandBufferCount = 1;

	submitInfo.waitSemaphoreCount = waitSemaphores.size();
	std::vector<VkSemaphore> semaphores;
	std::vector<VkPipelineStageFlags> stages;
	for (int i(0); i < waitSemaphores.size(); ++i)
	{
		semaphores.push_back(waitSemaphores[i]->getSemaphore());
		stages.push_back(waitSemaphores[i]->getPipelineStage());
	}
	submitInfo.pWaitSemaphores = semaphores.data();
	submitInfo.pWaitDstStageMask = stages.data();

	if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("Error : submit to compute queue");
}

void RayTracingShadows::resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkQueue graphicsQueue, ModelPBR* model, VkExtent2D extentOutput)
{
	m_textureTarget.cleanup(device);

	m_textureTarget.create(device, physicalDevice, extentOutput, VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_textureTarget.setImageLayout(device, commandPool, graphicsQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_SHADER_WRITE_BIT);

	m_rtDSG.reset();
	m_rtDSG = std::make_unique<nv_helpers_vk::DescriptorSetGenerator>();
	vkDestroyDescriptorSetLayout(device, m_rtDescriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device, m_rtDescriptorPool, nullptr);
	
	std::vector<VertexBuffer> vertexBuffers = model->getVertexBuffers();
	createRaytracingDescriptorSet(device, model->getImages(0), model->getSampler(), vertexBuffers[0].vertexBuffer, vertexBuffers[0].indexBuffer);

	fillCommandBuffer(extentOutput);
}

void RayTracingShadows::changeSampleCount(VkDevice device, unsigned sampleCount)
{
	m_uboParamsData.sampleCount = sampleCount;
	m_uboParams.updateData(device, &m_uboParamsData);
}

void RayTracingShadows::cleanup(VkDevice device, VkCommandPool commandPool)
{
	m_textureTarget.cleanup(device);
	m_command.cleanup(device, commandPool);
	vkDestroyDescriptorSetLayout(device, m_rtDescriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device, m_rtDescriptorPool, nullptr);
	m_accelerationStructure.cleanup(device);
	vkDestroyBuffer(device, m_shaderBindingTableBuffer, nullptr);
	vkFreeMemory(device, m_shaderBindingTableMem, nullptr);
	m_pipeline.cleanup(device);
	m_rtDSG.reset();
}

void RayTracingShadows::fillCommandBuffer(VkExtent2D extent)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(m_command.getCommandBuffer(0), &beginInfo);

	vkCmdBindPipeline(m_command.getCommandBuffer(0), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_pipeline.getPipeline());
	vkCmdBindDescriptorSets(m_command.getCommandBuffer(0), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
		m_pipeline.getPipelineLayout(), 0, 1, &m_rtDescriptorSet,
		0, nullptr);

	VkDeviceSize rayGenOffset = m_sbtGen.GetRayGenOffset();
	VkDeviceSize missOffset = m_sbtGen.GetMissOffset();
	VkDeviceSize missStride = m_sbtGen.GetMissEntrySize();
	VkDeviceSize hitGroupOffset = m_sbtGen.GetHitGroupOffset();
	VkDeviceSize hitGroupStride = m_sbtGen.GetHitGroupEntrySize();

	vkCmdTraceRaysNV(m_command.getCommandBuffer(0), m_shaderBindingTableBuffer, rayGenOffset,
		m_shaderBindingTableBuffer, missOffset, missStride,
		m_shaderBindingTableBuffer, hitGroupOffset, hitGroupStride,
		VK_NULL_HANDLE, 0, 0, extent.width, extent.height, 1);

	vkEndCommandBuffer(m_command.getCommandBuffer(0));
}

void RayTracingShadows::createRaytracingDescriptorSet(VkDevice device, std::vector<Image*> images, Sampler* sampler, VkBuffer vertexBuffer, VkBuffer indexBuffer)
{
	// Add the bindings to the resources
	// Top-level acceleration structure, usable by both the ray generation and the
	// closest hit (to shoot shadow rays)
	m_rtDSG->AddBinding(0, 1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
	// Raytracing output
	m_rtDSG->AddBinding(1, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_NV);
	// Camera information
	m_rtDSG->AddBinding(2, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_NV);
	// Scene data
	// Vertex buffer
	m_rtDSG->AddBinding(3, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
	// Index buffer
	m_rtDSG->AddBinding(4, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
	// Textures
	m_rtDSG->AddBinding(5, static_cast<uint32_t>(images.size()), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
	// Params
	m_rtDSG->AddBinding(6, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_NV);
	
	// Create the descriptor pool and layout
	m_rtDescriptorPool = m_rtDSG->GeneratePool(device);
	m_rtDescriptorSetLayout = m_rtDSG->GenerateLayout(device);

	// Generate the descriptor set
	m_rtDescriptorSet = m_rtDSG->GenerateSet(device, m_rtDescriptorPool, m_rtDescriptorSetLayout);

	// Bind the actual resources into the descriptor set
	// Top-level acceleration structure
	VkWriteDescriptorSetAccelerationStructureNV descriptorAccelerationStructureInfo;
	descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
	descriptorAccelerationStructureInfo.pNext = nullptr;
	descriptorAccelerationStructureInfo.accelerationStructureCount = 1;

	VkAccelerationStructureNV topLevelAS = m_accelerationStructure.getToLevelAccelerationStructure();
	descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAS;

	m_rtDSG->Bind(m_rtDescriptorSet, 0, { descriptorAccelerationStructureInfo });

	// Camera matrices
	VkDescriptorBufferInfo camInfo = {};
	camInfo.buffer = m_uboInvMVP.getUniformBuffer();
	camInfo.offset = 0;
	camInfo.range = sizeof(InvMvpUBO);

	m_rtDSG->Bind(m_rtDescriptorSet, 2, { camInfo });

	// Vertex buffer
	VkDescriptorBufferInfo vertexInfo = {};
	vertexInfo.buffer = vertexBuffer;
	vertexInfo.offset = 0;
	vertexInfo.range = VK_WHOLE_SIZE;

	m_rtDSG->Bind(m_rtDescriptorSet, 3, { vertexInfo });

	// Index buffer
	VkDescriptorBufferInfo indexInfo = {};
	indexInfo.buffer = indexBuffer;
	indexInfo.offset = 0;
	indexInfo.range = VK_WHOLE_SIZE;

	m_rtDSG->Bind(m_rtDescriptorSet, 4, { indexInfo });

	// Textures
	std::vector<VkDescriptorImageInfo> imageInfos;
	for (size_t i = 0; i < images.size(); ++i)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = images[i]->getImageView();
		imageInfo.sampler = sampler->getSampler();
		imageInfos.push_back(imageInfo);
	}
	if (!images.empty())
	{
		m_rtDSG->Bind(m_rtDescriptorSet, 5, imageInfos);
	}

	// Params
	VkDescriptorBufferInfo paramsInfo = {};
	paramsInfo.buffer = m_uboParams.getUniformBuffer();
	paramsInfo.offset = 0;
	paramsInfo.range = sizeof(m_uboParamsData);

	m_rtDSG->Bind(m_rtDescriptorSet, 6, { paramsInfo });

	// Output buffer
	VkDescriptorImageInfo descriptorOutputImageInfo;
	descriptorOutputImageInfo.sampler = nullptr;
	descriptorOutputImageInfo.imageView = m_textureTarget.getImageView();
	descriptorOutputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	m_rtDSG->Bind(m_rtDescriptorSet, 1, { descriptorOutputImageInfo });

	// Copy the bound resource handles into the descriptor set
	m_rtDSG->UpdateSetContents(device, m_rtDescriptorSet);
}

void RayTracingShadows::updateRaytracingRenderTarget(VkDevice device, VkImageView target)
{
	// Output buffer
	VkDescriptorImageInfo descriptorOutputImageInfo;
	descriptorOutputImageInfo.sampler = nullptr;
	descriptorOutputImageInfo.imageView = target;
	descriptorOutputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	m_rtDSG->Bind(m_rtDescriptorSet, 1, { descriptorOutputImageInfo });
	// Copy the bound resource handles into the descriptor set
	m_rtDSG->UpdateSetContents(device, m_rtDescriptorSet);
}

void RayTracingShadows::createShaderBindingTable(VkDevice device, VkPhysicalDevice physicalDevice)
{
	// Add the entry point, the ray generation program
	m_sbtGen.AddRayGenerationProgram(m_pipeline.getRayGenIndex(), {});
	// Add the miss shader for the camera rays
	m_sbtGen.AddMissProgram(m_pipeline.getMissIndex(), {});

	m_sbtGen.AddMissProgram(m_pipeline.getShadowMissIndex(), {});

	// For each instance, we will have 1 hit group for the camera rays.
	// When setting the instances in the top-level acceleration structure we indicated the index
	// of the hit group in the shader binding table that will be invoked.

	// Add the hit group defining the behavior upon hitting a surface with a camera ray
	m_sbtGen.AddHitGroup(m_pipeline.getHitGroupIndex(), {});

	m_sbtGen.AddHitGroup(m_pipeline.getShadowHitGroupIndex(), {});

	// Compute the required size for the SBT
	VkDeviceSize shaderBindingTableSize = m_sbtGen.ComputeSBTSize(getPhysicalDeviceRayTracingProperties(physicalDevice));

	// Allocate mappable memory to store the SBT
	createBuffer(device, physicalDevice, shaderBindingTableSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_shaderBindingTableBuffer, m_shaderBindingTableMem);

	// Generate the SBT using mapping. For further performance a staging buffer should be used, so
	// that the SBT is guaranteed to reside on GPU memory without overheads.
	m_sbtGen.Generate(device, m_pipeline.getPipeline(), m_shaderBindingTableBuffer,
		m_shaderBindingTableMem);
}
