#include "ComputePass.h"

void ComputePass::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool,
	VkExtent2D extent, VkExtent3D dispatchGroups, std::string computeShader,
	std::vector<std::pair<UniformBufferObject*, UniformBufferObjectLayout>> ubos, std::vector<std::pair<Texture*, TextureLayout>> textures,
	std::vector<Operation> operationsBefore, std::vector<Operation> operationsAfter)
{
	/* Create command buffer */
	m_command.allocateCommandBuffers(device, commandPool, 1);

	/* Create pipeline */
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayouts;
	for (int i(0); i < ubos.size(); ++i)
	{
		VkDescriptorSetLayoutBinding descriptorSetLayout = {};
		descriptorSetLayout.binding = ubos[i].second.binding;
		descriptorSetLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetLayout.descriptorCount = 1;
		descriptorSetLayout.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		descriptorSetLayout.pImmutableSamplers = nullptr;

		descriptorSetLayouts.push_back(descriptorSetLayout);
	}

	for (int i(0); i < textures.size(); ++i)
	{
		VkDescriptorSetLayoutBinding descriptorSetLayout = {};
		descriptorSetLayout.binding = textures[i].second.binding;
		descriptorSetLayout.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptorSetLayout.descriptorCount = 1;
		descriptorSetLayout.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		descriptorSetLayout.pImmutableSamplers = nullptr;

		descriptorSetLayouts.push_back(descriptorSetLayout);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	layoutInfo.pBindings = descriptorSetLayouts.data();

	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Error : create descriptor set layout");

	m_pipeline.initialize(device, computeShader, &descriptorSetLayout);

	VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	VkDescriptorSet descriptorSet;
	VkResult res = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Error : allocate descriptor set");

	// Descriptor Set
	std::vector<VkWriteDescriptorSet> descriptorWrites;

	std::vector<VkDescriptorBufferInfo> bufferInfo(ubos.size());
	for (int i(0); i < ubos.size(); ++i)
	{
		bufferInfo[i].buffer = ubos[i].first->getUniformBuffer();
		bufferInfo[i].offset = 0;
		bufferInfo[i].range = ubos[i].first->getSize();

		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = ubos[i].second.binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo[i];
		descriptorWrite.pNext = NULL;

		descriptorWrites.push_back(descriptorWrite);
	}

	std::vector<VkDescriptorImageInfo> imageInfo(textures.size());
	for (int i(0); i < textures.size(); ++i)
	{
		imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo[i].imageView = textures[i].first->getImageView();
		imageInfo[i].sampler = textures[i].first->getSampler();

		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = textures[i].second.binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo[i];
		descriptorWrite.pNext = NULL;

		descriptorWrites.push_back(descriptorWrite);
	}

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	/* Fill command buffer */
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(m_command.getCommandBuffer(0), &beginInfo);

	fillCommandBufferWithOperation(m_command.getCommandBuffer(0), operationsBefore);

	vkCmdBindPipeline(m_command.getCommandBuffer(0), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline.getPipeline());
	vkCmdBindDescriptorSets(m_command.getCommandBuffer(0), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline.getPipelineLayout(), 0, 1, &descriptorSet, 0, 0);
	uint32_t groupSizeX = extent.width % dispatchGroups.width != 0 ? extent.width / dispatchGroups.width + 1 : extent.width / dispatchGroups.width;
	uint32_t groupSizeY = extent.height % dispatchGroups.height != 0 ? extent.height / dispatchGroups.height + 1 : extent.height / dispatchGroups.height;
	vkCmdDispatch(m_command.getCommandBuffer(0), groupSizeX, groupSizeY, dispatchGroups.depth);

	fillCommandBufferWithOperation(m_command.getCommandBuffer(0), operationsAfter);

	vkEndCommandBuffer(m_command.getCommandBuffer(0));
}

void ComputePass::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> waitSemaphores, VkSemaphore signalSemaphore)
{
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

	if (vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("Error : submit to compute queue");
}

void ComputePass::cleanup(VkDevice device, VkCommandPool commandPool)
{
	m_command.cleanup(device, commandPool);
	m_pipeline.cleanup(device);
}

void ComputePass::fillCommandBufferWithOperation(VkCommandBuffer commandBuffer, std::vector<Operation> operations)
{
	for (int i(0); i < operations.size(); ++i)
	{
		int operationType = operations[i].getOperationType();

		if (operationType == OPERATION_TYPE_COPY_IMAGE)
		{
			VkImageCopy copyRegion = {};
			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };
			copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.dstSubresource.mipLevel = 0;
			copyRegion.dstSubresource.baseArrayLayer = 0;
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = { 0, 0, 0 };
			copyRegion.extent = { operations[i].getSourceImage()->getExtent().width, operations[i].getSourceImage()->getExtent().height, 1 };

			vkCmdCopyImage(commandBuffer,
				operations[i].getSourceImage()->getImage(), operations[i].getSourceLayout(),
				operations[i].getDestinationImage()->getImage(), operations[i].getDestinationLayout(),
				1, &copyRegion);
		}
		else if (operationType == OPERATION_TYPE_TRANSIT_IMAGE_LAYOUT)
		{
			Image::transitionImageLayoutUsingCommandBuffer(commandBuffer, operations[i].getSourceImage()->getImage(),
				operations[i].getSourceImage()->getFormat(), operations[i].getSourceLayout(),
				operations[i].getDestinationLayout(), 1, operations[i].getSourceStage(),
				operations[i].getDestinationStage(), 0);
		}
	}
}
