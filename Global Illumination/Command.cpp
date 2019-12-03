#include "Command.h"

Command::~Command()
{
}

bool Command::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	return true;
}

void Command::allocateCommandBuffers(VkDevice device, VkCommandPool commandPool, size_t size)
{
	m_commandBuffers.resize(size);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("Error : command buffer allocation");
}

void Command::fillCommandBuffer(VkDevice device, size_t commandBufferID, VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent, std::vector<VkClearValue> clearValues,
	std::vector<Renderer*> renderers)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(m_commandBuffers[commandBufferID], &beginInfo);

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(m_commandBuffers[commandBufferID], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	for (int i(0); i < renderers.size(); ++i)
	{
		vkCmdBindPipeline(m_commandBuffers[commandBufferID], VK_PIPELINE_BIND_POINT_GRAPHICS, renderers[i]->getPipeline());

		const VkDeviceSize offsets[1] = { 0 };

		std::vector<std::pair<VertexBuffer, VkDescriptorSet>> meshesToRender = renderers[i]->getMeshes();
		for (int j(0); j < meshesToRender.size(); ++j)
		{
			vkCmdBindVertexBuffers(m_commandBuffers[commandBufferID], 0, 1, &meshesToRender[j].first.vertexBuffer, offsets);
			vkCmdBindIndexBuffer(m_commandBuffers[commandBufferID], meshesToRender[j].first.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(m_commandBuffers[commandBufferID], VK_PIPELINE_BIND_POINT_GRAPHICS,
				renderers[i]->getPipelineLayout(), 0, 1, &meshesToRender[j].second, 0, nullptr);

			vkCmdDrawIndexed(m_commandBuffers[commandBufferID], meshesToRender[j].first.nbIndices, 1, 0, 0, 0);
		}

	}

	vkCmdEndRenderPass(m_commandBuffers[commandBufferID]);

	if (vkEndCommandBuffer(m_commandBuffers[commandBufferID]) != VK_SUCCESS)
		throw std::runtime_error("Error : end command buffer");
}

void Command::fillCommandBuffer(VkDevice device, size_t commandBufferID, Operation& operation)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(m_commandBuffers[commandBufferID], &beginInfo);

	std::vector<CopyImageOperation> copyImageOperations = operation.getCopyImageOperation();
	for (int i(0); i < copyImageOperations.size(); ++i)
	{
		VkImageCopy copyRegion = {};
		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffset = { 0, 0, 0 };
		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.mipLevel = 0;
		copyRegion.dstSubresource.baseArrayLayer = 0;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffset = { 0, 0, 0 };
		copyRegion.extent = { copyImageOperations[i].srcImage->getExtent().width, copyImageOperations[i].srcImage->getExtent().height, 1 };

		vkCmdCopyImage(m_commandBuffers[commandBufferID],
			copyImageOperations[i].srcImage->getImage(), copyImageOperations[i].srcImage->getImageLayout(),
			copyImageOperations[i].dstImage->getImage(), copyImageOperations[i].dstImage->getImageLayout(),
			1, &copyRegion);
	}

	if (vkEndCommandBuffer(m_commandBuffers[commandBufferID]) != VK_SUCCESS)
		throw std::runtime_error("Error : end command buffer");
}

void Command::submit(VkDevice device, VkQueue graphicsQueue, std::vector<Semaphore*> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, size_t commandBufferID)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submitInfo.signalSemaphoreCount = signalSemaphores.size();
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	VkCommandBuffer commandBuffer = m_commandBuffers[0];
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

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("Error : submit to graphics queue");
}

void Command::cleanup(VkDevice device, VkCommandPool commandPool)
{
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
}
