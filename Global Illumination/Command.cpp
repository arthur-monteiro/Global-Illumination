#include "Command.h"

Command::~Command()
{
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

		std::vector<std::tuple<VertexBuffer, InstanceBuffer, VkDescriptorSet>> meshesInstancied = renderers[i]->getMeshesInstancied();
		for(int j(0); j < meshesInstancied.size(); ++j)
		{
			vkCmdBindVertexBuffers(m_commandBuffers[commandBufferID], 0, 1, &std::get<0>(meshesInstancied[j]).vertexBuffer, offsets);
			vkCmdBindIndexBuffer(m_commandBuffers[commandBufferID], std::get<0>(meshesInstancied[j]).indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindVertexBuffers(m_commandBuffers[commandBufferID], 1, 1, &std::get<1>(meshesInstancied[j]).instanceBuffer, offsets);

			vkCmdBindDescriptorSets(m_commandBuffers[commandBufferID], VK_PIPELINE_BIND_POINT_GRAPHICS,
				renderers[i]->getPipelineLayout(), 0, 1, &std::get<2>(meshesInstancied[j]), 0, nullptr);

			vkCmdDrawIndexed(m_commandBuffers[commandBufferID], std::get<0>(meshesInstancied[j]).nbIndices, std::get<1>(meshesInstancied[j]).nInstances, 0, 0, 0);
		}
	}

	vkCmdEndRenderPass(m_commandBuffers[commandBufferID]);

	if (vkEndCommandBuffer(m_commandBuffers[commandBufferID]) != VK_SUCCESS)
		throw std::runtime_error("Error : end command buffer");
}

void Command::fillCommandBuffer(VkDevice device, size_t commandBufferID, std::vector<Operation> operations)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(m_commandBuffers[commandBufferID], &beginInfo);

	for(int i(0); i < operations.size(); ++i)
    {
	    int operationType = operations[i].getOperationType();

	    if(operationType == OPERATION_TYPE_COPY_IMAGE)
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
            copyRegion.extent = { operations[i].getSourceImage()->getExtent().width, operations[i].getSourceImage()->getExtent().height, 1 };

            vkCmdCopyImage(m_commandBuffers[commandBufferID],
                           operations[i].getSourceImage()->getImage(), operations[i].getSourceLayout(),
                           operations[i].getDestinationImage()->getImage(), operations[i].getDestinationLayout(),
                           1, &copyRegion);
        }
	    else if(operationType == OPERATION_TYPE_TRANSIT_IMAGE_LAYOUT)
        {
	        Image::transitionImageLayoutUsingCommandBuffer(m_commandBuffers[commandBufferID], operations[i].getSourceImage()->getImage(),
                                                           operations[i].getSourceImage()->getFormat(), operations[i].getSourceLayout(),
                                                           operations[i].getDestinationLayout(), 1, operations[i].getSourceStage(),
                                                           operations[i].getDestinationStage(), 0);
        }
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

	VkCommandBuffer commandBuffer = m_commandBuffers[commandBufferID];
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
	m_commandBuffers.clear();
}
