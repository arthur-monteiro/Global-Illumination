#include "LoadingManager.h"

LoadingManager::~LoadingManager()
= default;

bool LoadingManager::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<Image*> swapChainImages)
{
    std::vector<Attachment> attachments(2);
    attachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    attachments[1].initialize(swapChainImages[0]->getFormat(), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    m_mainRenderPass.initialize(device, physicalDevice, surface, attachments, swapChainImages);

    for(int i(0); i < swapChainImages.size(); ++i)
    {
        std::vector<VkClearValue> clearValues(2);
        clearValues[0] = { 1.0f };
        clearValues[1] = { 0.0f, 0.0f, 0.0f, 1.0f };
        m_mainRenderPass.fillCommandBuffer(device, i, clearValues);
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
