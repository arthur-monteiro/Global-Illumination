#include "ComputePass.h"

void ComputePass::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkQueue computeQueue,
	VkExtent2D extent, VkExtent3D dispatchGroups, std::string computeShader,
	std::vector<std::pair<UniformBufferObject*, UniformBufferObjectLayout>> ubos, std::vector<std::pair<Texture*, TextureLayout>> textures)
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
		imageInfo[i].imageLayout = textures[i].first->getImageLayout();
		imageInfo[i].imageView = textures[i].first->getImageView();
		imageInfo[i].sampler = textures[i].first->getSampler();

		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = textures[i].second.binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptorWrite.descriptorCount = 1;

		descriptorWrites.push_back(descriptorWrite);
	}

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	/* Fill command buffer */
	vkQueueWaitIdle(computeQueue);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(m_command.getCommandBuffer(0), &beginInfo);

	vkCmdBindPipeline(m_command.getCommandBuffer(0), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline.getPipeline());
	vkCmdBindDescriptorSets(m_command.getCommandBuffer(0), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline.getPipelineLayout(), 0, 1, &descriptorSet, 0, 0);
	uint32_t groupSizeX = extent.width % dispatchGroups.width != 0 ? extent.width / dispatchGroups.width + 1 : extent.width / dispatchGroups.width;
	uint32_t groupSizeY = extent.height % dispatchGroups.height != 0 ? extent.height / dispatchGroups.height + 1 : extent.height / dispatchGroups.height;
	vkCmdDispatch(m_command.getCommandBuffer(0), groupSizeX, groupSizeY, dispatchGroups.depth);

	vkEndCommandBuffer(m_command.getCommandBuffer(0));

	/* Semaphore creation */
	m_renderCompleteSemaphore.initialize(device);
}

void ComputePass::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> waitSemaphores)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore signalSemaphores[] = { m_renderCompleteSemaphore.getSemaphore() };
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

void ComputePass::cleanup(VkDevice device)
{
}