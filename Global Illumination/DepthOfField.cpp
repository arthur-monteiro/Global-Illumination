#include "DepthOfField.h"

#include <utility>

void DepthOfField::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool computeCommandPool, VkCommandPool graphicsCommandPool,
                              VkDescriptorPool descriptorPool, VkQueue computeQueue, VkQueue graphicsQueue, Texture* inputViewPos, Texture* inputColor)
{
	// Depth + Circle of confusion pass
	{
		m_outputDepthBlurTexture.create(device, physicalDevice, inputViewPos->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R16G16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_outputDepthBlurTexture.setImageLayout(device, computeCommandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		m_outputDepthBlurTexture.createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);

		TextureLayout textureViewPosInputLayout{};
		textureViewPosInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureViewPosInputLayout.binding = 0;

		TextureLayout textureOutputLayout{};
		textureOutputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureOutputLayout.binding = 1;

		UniformBufferObjectLayout uboLayout{};
		uboLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		uboLayout.binding = 2;

		m_depthBlurUBO.initialize(device, physicalDevice, &m_depthBlurUBOData, sizeof(DepthBlurUBO));

		std::string shaderPath = "Shaders/postProcess/depthOfField/depthBlur.spv";

		m_depthBlurPass.initialize(device, physicalDevice, computeCommandPool, descriptorPool, inputViewPos->getImage()->getExtent(), { 16, 16, 1 }, shaderPath,
			{ { &m_depthBlurUBO, uboLayout} },
			{ { inputViewPos, textureViewPosInputLayout}, { &m_outputDepthBlurTexture, textureOutputLayout} },
			{}, {});

		m_depthBlurSemaphore.initialize(device);
		m_depthBlurSemaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	// Bokeh Point Generation
	{
		VkExtent2D extent = inputViewPos->getImage()->getExtent();
		
		// Create bokeh point buffer
		VkDeviceSize bokehPointBufferSize = extent.width * extent.height * sizeof(BokehPoint);
		createBuffer(device, physicalDevice, bokehPointBufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_bokehPointsBuffer, m_bokehPointsBufferMemory);

		// Create indirect draw command buffer
		m_indirectBokehPointsDrawCommand.instanceCount = 1; // this parameter will not change
		m_indirectBokehPointsDrawCommand.firstInstance = 0;
		m_indirectBokehPointsDrawCommand.firstIndex = 0;
		m_indirectBokehPointsDrawCommand.indexCount = 0; // set to 0
		m_indirectBokehPointsDrawCommand.vertexOffset = 0;

		createBuffer(device, physicalDevice, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_indirectBokehPointsDrawCommandBufferDefault, m_indirectBokehPointsDrawCommandBufferMemoryDefault);

		void* tData;
		vkMapMemory(device, m_indirectBokehPointsDrawCommandBufferMemoryDefault, 0, sizeof(VkDrawIndexedIndirectCommand), 0, &tData);
		std::memcpy(tData, &m_indirectBokehPointsDrawCommand, sizeof(VkDrawIndexedIndirectCommand));
		vkUnmapMemory(device, m_indirectBokehPointsDrawCommandBufferMemoryDefault);

		createBuffer(device, physicalDevice, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_indirectBokehPointsDrawCommandBuffer, m_indirectBokehPointsDrawCommandBufferMemory);

		copyBuffer(device, computeCommandPool, computeQueue, m_indirectBokehPointsDrawCommandBufferDefault, m_indirectBokehPointsDrawCommandBuffer, sizeof(VkDrawIndexedIndirectCommand));

		// Create render pass : we'll simply render a full screen quad
		m_bokehPointGenerationAttachements.resize(2);
		m_bokehPointGenerationAttachements[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		m_bokehPointGenerationAttachements[1].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

		m_bokehPointGenerationRenderPass.initialize(device, physicalDevice, graphicsCommandPool, m_bokehPointGenerationAttachements, { extent });

		// Crete mesh : a quad
		m_quad.loadFromVertices(device, physicalDevice, computeCommandPool, computeQueue, {
			{ glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f) }, // bot left
			{ glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f) }, // top left
			{ glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 0.0f) }, // bot right
			{ glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f) } // top right
			}, { 0, 1, 2, 1, 3, 2 });

		// Create renderer
		BufferLayout indirectCmdDrawBufferLayout;
		indirectCmdDrawBufferLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
		indirectCmdDrawBufferLayout.binding = 0;
		indirectCmdDrawBufferLayout.size = sizeof(VkDrawIndexedIndirectCommand);

		BufferLayout bokehPointBufferLayout;
		bokehPointBufferLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
		bokehPointBufferLayout.binding = 1;
		bokehPointBufferLayout.size = bokehPointBufferSize;

		TextureLayout textureColorLayout;
		textureColorLayout.binding = 2;
		textureColorLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;

		TextureLayout textureDepthBlurLayout;
		textureDepthBlurLayout.binding = 3;
		textureDepthBlurLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;

		UniformBufferObjectLayout uboLayout;
		uboLayout.binding = 4;
		uboLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;

		m_bokehPointGenerationRenderer.initialize(device, "Shaders/postProcess/depthOfField/vertexBokehPointGeneration.spv", 
			"Shaders/postProcess/depthOfField/fragmentBokehPointGeneration.spv", { Vertex2DTextured::getBindingDescription(0) }, 
			Vertex2DTextured::getAttributeDescriptions(0), { uboLayout }, { textureColorLayout,  textureDepthBlurLayout }, {}, {},
			{ indirectCmdDrawBufferLayout, bokehPointBufferLayout }, { true });

		m_bokehPointGenerationParams.params.x = 3.5f; // brightness threshold
		m_bokehPointGenerationParams.params.y = 0.5f; // blur threshold
		m_bokehPointsGenerationUBO.initialize(device, physicalDevice, &m_bokehPointGenerationParams, sizeof(BokehPointGenerationParams));
		
		m_bokehPointGenerationRenderer.addMesh(device, descriptorPool, m_quad.getVertexBuffer(), { { &m_bokehPointsGenerationUBO, uboLayout } }, { { inputColor, textureColorLayout }, { &m_outputDepthBlurTexture, textureDepthBlurLayout } }, {}, {},
			{
				{ m_indirectBokehPointsDrawCommandBuffer, indirectCmdDrawBufferLayout },
				{ m_bokehPointsBuffer, bokehPointBufferLayout }
			});

		// Fill command buffer
		m_bokehPointGenerationClearValues.resize(2);
		m_bokehPointGenerationClearValues[0] = { 1.0f };
		m_bokehPointGenerationClearValues[1] = { 0.0f, 1.0f, 1.0f, 1.0f };

		m_bokehPointGenerationRenderPass.fillCommandBuffer(device, 0, m_bokehPointGenerationClearValues, { &m_bokehPointGenerationRenderer }, VK_SAMPLE_COUNT_1_BIT, false);

		/*VkBufferCopy copyRegion = {};
		copyRegion.size = sizeof(VkDrawIndexedIndirectCommand);
		vkCmdCopyBuffer(m_bokehPointGenerationRenderPass.getCommandBuffer(0), m_indirectBokehPointsDrawCommandBufferDefault, m_indirectBokehPointsDrawCommandBuffer, 1, 
			&copyRegion);*/

		m_bokehPointGenerationRenderPass.endCommandBuffer(0);
	}

	// Bokeh shape draw
	{
		VkExtent2D extent = inputViewPos->getImage()->getExtent();

		m_bokedPointDrawAttachments.resize(2);
		m_bokedPointDrawAttachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		m_bokedPointDrawAttachments[1].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

		m_bokedPointDrawRenderPass.initialize(device, physicalDevice, graphicsCommandPool, m_bokedPointDrawAttachments, { extent });

		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(BokehPoint);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(BokehPoint, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(BokehPoint, blurAndColor);

		m_bokehShapeTexture.createFromFile(device, physicalDevice, graphicsCommandPool, graphicsQueue, "Textures/bokeh_hex.png");
		m_bokehShapeTexture.createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, m_bokehShapeTexture.getMipLevels(), VK_FILTER_LINEAR);

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		// Bokeh shape
		{
			VkDescriptorSetLayoutBinding imageSamplerLayoutBinding = {};
			imageSamplerLayoutBinding.binding = 0;
			imageSamplerLayoutBinding.descriptorCount = static_cast<uint32_t>(1);
			imageSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imageSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			imageSamplerLayoutBinding.pImmutableSamplers = nullptr;

			bindings.push_back(imageSamplerLayoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_bokehPointDrawDescriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("Error : create descriptor set layout");

		m_bokehPointDrawPipeline.initialize(device, m_bokedPointDrawRenderPass.getRenderPass(), "Shaders/postProcess/depthOfField/vertexBokehPoint.spv", "Shaders/postProcess/depthOfField/geometryBokehPoint.spv",
			"Shaders/postProcess/depthOfField/fragmentBokehPoint.spv", { bindingDescription }, attributeDescriptions, inputViewPos->getImage()->getExtent(), VK_SAMPLE_COUNT_1_BIT,
			{ true }, &m_bokehPointDrawDescriptorSetLayout, VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_FALSE, true);

		// Descriptor set
		{
			VkDescriptorSetLayout layouts[] = { m_bokehPointDrawDescriptorSetLayout };
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = layouts;
			
			VkResult res = vkAllocateDescriptorSets(device, &allocInfo, &m_bokehPointDrawDescriptorSet);
			if (res != VK_SUCCESS)
				throw std::runtime_error("Error : allocate descriptor set");

			VkDescriptorImageInfo imageSamplerInfo;
			imageSamplerInfo.imageLayout = m_bokehShapeTexture.getImageLayout();
			imageSamplerInfo.imageView = m_bokehShapeTexture.getImageView();
			imageSamplerInfo.sampler = m_bokehShapeTexture.getSampler();

			std::vector<VkWriteDescriptorSet> descriptorWrites;
			VkWriteDescriptorSet descriptorWrite;
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_bokehPointDrawDescriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageSamplerInfo;
			descriptorWrite.pNext = NULL;

			descriptorWrites.push_back(descriptorWrite);

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = graphicsCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(1);

		if (vkAllocateCommandBuffers(device, &allocInfo, &m_bokehPointDrawCommandBuffer) != VK_SUCCESS)
			throw std::runtime_error("Error : command buffer allocation");

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(m_bokehPointDrawCommandBuffer, &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_bokedPointDrawRenderPass.getRenderPass();
		renderPassInfo.framebuffer = m_bokedPointDrawRenderPass.getFramebuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = inputViewPos->getImage()->getExtent();

		m_bokedPointDrawClearValues.resize(2);
		m_bokedPointDrawClearValues[0] = { 1.0f };
		m_bokedPointDrawClearValues[1] = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		renderPassInfo.clearValueCount = static_cast<uint32_t>(m_bokedPointDrawClearValues.size());
		renderPassInfo.pClearValues = m_bokedPointDrawClearValues.data();

		vkCmdBeginRenderPass(m_bokehPointDrawCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_bokehPointDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_bokehPointDrawPipeline.getPipeline());

		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(m_bokehPointDrawCommandBuffer, 0, 1, &m_bokehPointsBuffer, offsets);
		vkCmdBindDescriptorSets(m_bokehPointDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_bokehPointDrawPipeline.getPipelineLayout(), 0, 1, &m_bokehPointDrawDescriptorSet, 0, nullptr);

		vkCmdDrawIndirect(m_bokehPointDrawCommandBuffer, m_indirectBokehPointsDrawCommandBuffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));
		//vkCmdDraw(m_commandBuffer, 4, 1, 0, 0);
		
		vkCmdEndRenderPass(m_bokehPointDrawCommandBuffer);

		VkBufferCopy copyRegion = {};
		copyRegion.size = sizeof(VkDrawIndexedIndirectCommand);
		vkCmdCopyBuffer(m_bokehPointDrawCommandBuffer, m_indirectBokehPointsDrawCommandBufferDefault, m_indirectBokehPointsDrawCommandBuffer, 1,
			&copyRegion);

		if (vkEndCommandBuffer(m_bokehPointDrawCommandBuffer) != VK_SUCCESS)
			throw std::runtime_error("Error : end command buffer");

		m_bokehPointDrawSemaphore.initialize(device);
		m_bokehPointDrawSemaphore.setPipelineStage(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		m_bokehPointDrawRenderPassResultTexture.createFromImage(device, m_bokedPointDrawRenderPass.getImages(0)[1]);
		m_bokehPointDrawRenderPassResultTexture.setImageLayout(device, graphicsCommandPool, graphicsQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		m_bokehPointDrawBlur.initialize(device, physicalDevice, computeCommandPool, descriptorPool, computeQueue, &m_bokehPointDrawRenderPassResultTexture, 2, "Shaders/blur/vertical32.spv", "Shaders/blur/horizontal32.spv");
	}

	// Blur
	{
		m_textureBlur.create(device, physicalDevice, inputViewPos->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_textureBlur.setImageLayout(device, computeCommandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		
		m_blur.initialize(device, physicalDevice, computeCommandPool, descriptorPool, computeQueue, inputColor, 10, "Shaders/blur/vertical32.spv", "Shaders/blur/horizontal32.spv", &m_textureBlur);
	}

	// Blur with a poisson disk
	{
		for(int i(0); i < m_poissonBlur.size(); ++i)
		{
			m_outputPoissonBlurTexture[i].create(device, physicalDevice, inputViewPos->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
			m_outputPoissonBlurTexture[i].setImageLayout(device, computeCommandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			TextureLayout inputColorLayout;
			inputColorLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			inputColorLayout.binding = 0;

			TextureLayout inputDepthBlurLayout;
			inputDepthBlurLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			inputDepthBlurLayout.binding = 1;

			TextureLayout outputLayout;
			outputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			outputLayout.binding = 2;

			m_poissonBlur[i].initialize(device, physicalDevice, computeCommandPool, descriptorPool, inputColor->getImage()->getExtent(), { 16, 16, 1 }, "Shaders/postProcess/depthOfField/poissonDisk.spv", {},
				{ { i == 0 ? inputColor : &m_outputPoissonBlurTexture[i - 1], inputColorLayout }, { &m_outputDepthBlurTexture, inputDepthBlurLayout }, { &m_outputPoissonBlurTexture[i], outputLayout } },
				{}, {});

			m_poissonBlurSemaphore[i].initialize(device);
			m_poissonBlurSemaphore[i].setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		}
	}

	// Merge
	{
		m_outputTextureMerge.create(device, physicalDevice, inputViewPos->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_outputTextureMerge.setImageLayout(device, computeCommandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		if(m_usePoissonDiskSampling)
		{
			TextureLayout colorInputLayout{};
			colorInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			colorInputLayout.binding = 0;

			TextureLayout bokehPointInputLayout;
			bokehPointInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			bokehPointInputLayout.binding = 1;

			TextureLayout textureOutputLayout{};
			textureOutputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			textureOutputLayout.binding = 2;

			m_merge.initialize(device, physicalDevice, computeCommandPool, descriptorPool, inputColor->getImage()->getExtent(), { 16, 16, 1 }, "Shaders/postProcess/depthOfField/mergePoisson.spv", {},
				{ { &m_outputPoissonBlurTexture[1], colorInputLayout }, { &m_bokehPointDrawRenderPassResultTexture, bokehPointInputLayout }, { &m_outputTextureMerge, textureOutputLayout } },
				{}, {});
		}

		else if(m_useBlur)
		{
			TextureLayout colorNoBlurInputLayout{};
			colorNoBlurInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			colorNoBlurInputLayout.binding = 0;

			TextureLayout colorBlurInputLayout;
			colorBlurInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			colorBlurInputLayout.binding = 1;

			TextureLayout inputDepthBlurLayout;
			inputDepthBlurLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			inputDepthBlurLayout.binding = 2;

			TextureLayout bokehPointInputLayout;
			bokehPointInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			bokehPointInputLayout.binding = 3;

			TextureLayout textureOutputLayout{};
			textureOutputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			textureOutputLayout.binding = 4;

			m_merge.initialize(device, physicalDevice, computeCommandPool, descriptorPool, inputColor->getImage()->getExtent(), { 16, 16, 1 }, "Shaders/postProcess/depthOfField/merge.spv", {},
				{ { inputColor, colorNoBlurInputLayout }, { &m_textureBlur, colorBlurInputLayout}, { &m_outputDepthBlurTexture, inputDepthBlurLayout } , { &m_bokehPointDrawRenderPassResultTexture, bokehPointInputLayout }, { &m_outputTextureMerge, textureOutputLayout } },
				{}, {});
		}

		m_mergeSemaphore.initialize(device);
		m_mergeSemaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	m_isReady = true;
}

void DepthOfField::submit(VkDevice device, VkQueue computeQueue, VkQueue graphicsQueue, std::vector<Semaphore*> semaphoresToWait)
{
	if (m_useBlur)
	{
		m_blur.submit(device, computeQueue, semaphoresToWait);
		m_depthBlurPass.submit(device, computeQueue, { m_blur.getSemaphore() }, m_depthBlurSemaphore.getSemaphore());
	}
	else
		m_depthBlurPass.submit(device, computeQueue, semaphoresToWait, m_depthBlurSemaphore.getSemaphore());
	
	if (m_needUpdateDepthBlurUBO)
		m_depthBlurUBO.updateData(device, &m_depthBlurUBOData);

	if(m_usePoissonDiskSampling)
	{
		m_poissonBlur[0].submit(device, computeQueue, { &m_depthBlurSemaphore }, m_poissonBlurSemaphore[0].getSemaphore());
		m_poissonBlur[1].submit(device, computeQueue, { &m_poissonBlurSemaphore[0] }, m_poissonBlurSemaphore[1].getSemaphore());
	}

	if (m_needUpdateBokehPointsGenerationUBO)
		m_bokehPointsGenerationUBO.updateData(device, &m_bokehPointGenerationParams);

	if(m_drawBokehPoints)
	{
		m_bokehPointGenerationRenderPass.submit(device, graphicsQueue, 0, { m_usePoissonDiskSampling ? &m_poissonBlurSemaphore[1] : &m_depthBlurSemaphore });
		// Bokeh point draw
		{
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore signalSemaphores[] = { m_bokehPointDrawSemaphore.getSemaphore() };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			submitInfo.pCommandBuffers = &m_bokehPointDrawCommandBuffer;
			submitInfo.commandBufferCount = 1;

			submitInfo.waitSemaphoreCount = 1;
			VkSemaphore semaphoreToWait = m_bokehPointGenerationRenderPass.getRenderCompleteSemaphore()->getSemaphore();
			submitInfo.pWaitSemaphores = &semaphoreToWait;
			VkPipelineStageFlags stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			submitInfo.pWaitDstStageMask = &stage;

			if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
				throw std::runtime_error("Error : submit to graphics queue");
		}
		m_bokehPointDrawBlur.submit(device, computeQueue, { &m_bokehPointDrawSemaphore });

		m_merge.submit(device, computeQueue, { m_bokehPointDrawBlur.getSemaphore() }, m_mergeSemaphore.getSemaphore());
	}
	else if(m_useBlur)
	{
		m_merge.submit(device, computeQueue, { &m_depthBlurSemaphore }, m_mergeSemaphore.getSemaphore());
	}
}

void DepthOfField::cleanup(VkDevice device, VkCommandPool computeCommandPool, VkCommandPool graphicsCommandPool, VkDescriptorPool descriptorPool)
{
	m_depthBlurPass.cleanup(device, computeCommandPool);
	m_outputDepthBlurTexture.cleanup(device);
	m_depthBlurSemaphore.cleanup(device);

	m_bokehPointGenerationRenderPass.cleanup(device, graphicsCommandPool);
	vkDestroyBuffer(device, m_bokehPointsBuffer, nullptr);
	m_bokehPointGenerationRenderer.cleanup(device, descriptorPool);

	vkDestroyBuffer(device, m_indirectBokehPointsDrawCommandBufferDefault, nullptr);
	vkDestroyBuffer(device, m_indirectBokehPointsDrawCommandBuffer, nullptr);

	m_quad.cleanup(device);

	m_bokedPointDrawRenderPass.cleanup(device, graphicsCommandPool);
	m_bokehPointDrawPipeline.cleanup(device);
	vkFreeCommandBuffers(device, graphicsCommandPool, 1, &m_bokehPointDrawCommandBuffer);
	m_bokehPointDrawSemaphore.cleanup(device);

	m_blur.cleanup(device, computeCommandPool);
	m_textureBlur.cleanup(device);

	m_merge.cleanup(device, computeCommandPool);
	m_outputTextureMerge.cleanup(device);
	m_mergeSemaphore.cleanup(device);

	m_isReady = false;
}

void DepthOfField::setBokehActivation(bool activate)
{
	m_drawBokehPoints = activate;
}

void DepthOfField::setBokehThresholdBrightness(float brightnessThreshold)
{
	m_bokehPointGenerationParams.params.x = brightnessThreshold;
	m_needUpdateBokehPointsGenerationUBO = true;
}

void DepthOfField::setBokehThresholdBlur(float blurThreshold)
{
	m_bokehPointGenerationParams.params.y = blurThreshold;
	m_needUpdateBokehPointsGenerationUBO = true;
}

void DepthOfField::setCenterFocusPoint(bool centered)
{
	if (centered)
		m_depthBlurUBOData.focusPoint = glm::vec4(-1.0f);
	else
		m_depthBlurUBOData.focusPoint = m_oldFocusPoint;
	m_needUpdateDepthBlurUBO = true;
}

void DepthOfField::setFocusPoint(glm::vec4 focusPoint)
{
	m_depthBlurUBOData.focusPoint = focusPoint;
	m_oldFocusPoint = focusPoint;
	m_needUpdateDepthBlurUBO = true;
}
