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

void Command::fillCommandBuffer(VkDevice device, size_t commandBufferID, VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent, std::vector<VkClearValue> clearValues)
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

	vkCmdEndRenderPass(m_commandBuffers[commandBufferID]);

	if (vkEndCommandBuffer(m_commandBuffers[commandBufferID]) != VK_SUCCESS)
		throw std::runtime_error("Error : end command buffer");
}
