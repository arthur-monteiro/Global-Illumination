#include "ComputePass.h"

void ComputePass::initialize(Vulkan* vk, VkExtent2D extent, VkExtent3D dispatchGroups, std::string computeShader)
{
	/* Create image */
	m_resultImage.create(vk, extent);

	/* Create command buffer */
	{
		m_commandPool = vk->createComputeCommandPool();

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(vk->getDevice(), &allocInfo, &m_commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("Error : command buffer allocation for compute shader");
	}

	/* Create pipeline */
	VkDescriptorSetLayoutBinding inputImageLayoutBinding = {};
	inputImageLayoutBinding.binding = 0;
	inputImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	inputImageLayoutBinding.descriptorCount = 1;
	inputImageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	inputImageLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding outputImageLayoutBinding = {};
	outputImageLayoutBinding.binding = 1;
	outputImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	outputImageLayoutBinding.descriptorCount = 1;
	outputImageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	outputImageLayoutBinding.pImmutableSamplers = nullptr;

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayouts = { inputImageLayoutBinding, outputImageLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	layoutInfo.pBindings = descriptorSetLayouts.data();

	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(vk->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Erreur : descriptor set layout");

	m_pipeline.intialize(vk, computeShader, &descriptorSetLayout);

	std::array<VkDescriptorPoolSize, 3> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1024;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1024;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[2].descriptorCount = 1024;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1024;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(vk->getDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Error : descriptor pool creation");

	VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	VkDescriptorSet descriptorSet;
	VkResult res = vkAllocateDescriptorSets(vk->getDevice(), &allocInfo, &descriptorSet);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Erreur : allocation descriptor set");

	/* Fill command buffer */
	vkQueueWaitIdle(vk->getComputeQueue());

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

	vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline.getComputePipeline());
	vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline.getPipelineLayout(), 0, 1, &descriptorSet, 0, 0);

	vkCmdDispatch(m_commandBuffer, extent.width / dispatchGroups.width, extent.width / dispatchGroups.height, dispatchGroups.depth);

	vkEndCommandBuffer(m_commandBuffer);

	/* Semaphore creation */
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(vk->getDevice(), &semaphoreInfo, nullptr, &m_renderCompleteSemaphore) != VK_SUCCESS)
		throw std::runtime_error("Error : semaphore creation");
}

void ComputePass::setSemaphoreToWait(VkDevice device, std::vector<Semaphore> semaphores)
{
	m_needToWaitSemaphores.clear();
	m_needToWaitStages.clear();

	for (int i(0); i < semaphores.size(); ++i)
	{
		m_needToWaitSemaphores.push_back(semaphores[i].semaphore);
		m_needToWaitStages.push_back(semaphores[i].stage);
	}
}
