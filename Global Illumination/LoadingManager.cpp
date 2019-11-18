#include "LoadingManager.h"

LoadingManager::~LoadingManager()
= default;

bool LoadingManager::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, std::vector<Image*> swapChainImages)
{
	// Command Pool
	m_uniqueCommandPool.initialize(device, physicalDevice, surface);

	// Quad
	m_quad.addMeshFromVertices(device, physicalDevice, m_uniqueCommandPool.getCommandPool(), graphicsQueue,
		{
			{ glm::vec2(-1.0f, -1.0f) }, // bot left
			{ glm::vec2(-1.0f, 1.0f) }, // top left
			{ glm::vec2(1.0f, -1.0f) }, // bot right
			{ glm::vec2(1.0f, 1.0f) } // top right
		},
		{
			0, 1, 2,
			1, 3, 2
		});

	// Quad renderer
	m_quadRenderer.initialize(device, "Shaders/loading/quadVertex.spv", "Shaders/loading/quadFragment.spv");
	m_quadRenderer.addModel(&m_quad);

	/* Main Render Pass*/
	// Attachments -> depth + color
    std::vector<Attachment> attachments(2);
    attachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    attachments[1].initialize(swapChainImages[0]->getFormat(), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	// Init with swapchain images
	m_mainRenderPass.initialize(device, physicalDevice, surface, m_uniqueCommandPool.getCommandPool(), attachments, swapChainImages);

	// Fill command buffers
    for(int i(0); i < swapChainImages.size(); ++i)
    {
        std::vector<VkClearValue> clearValues(2);
        clearValues[0] = { 1.0f };
        clearValues[1] = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_mainRenderPass.fillCommandBuffer(device, i, clearValues, { &m_quadRenderer });
    }

	return true;
}

bool LoadingManager::submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, const Semaphore& imageAvailableSemaphore)
{
    m_mainRenderPass.submit(device, graphicsQueue, swapChainImageIndex, { imageAvailableSemaphore });

    return true;
}

VkSemaphore LoadingManager::getLastRenderFinishedSemaphore()
{
    return m_mainRenderPass.getRenderCompleteSemaphore();
}
