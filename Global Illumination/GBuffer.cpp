#include "GBuffer.h"

GBuffer::~GBuffer()
{

}

bool GBuffer::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool commandPool, VkExtent2D extent)
{
    /* Main Render Pass */
    // Attachments -> depth + albedo + normal + (rougness + metal + ao)
    m_attachments.resize(4);
    m_attachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_attachments[1].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    m_attachments[2].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    m_attachments[3].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    m_renderPass.initialize(device, physicalDevice, surface, commandPool, m_attachments, { extent });

    return true;
}

bool GBuffer::submit(VkDevice device, VkQueue graphicsQueue)
{
    return false;
}

void GBuffer::resize(VkDevice device, VkPhysicalDevice physicalDevice, int width, int height)
{

}

void GBuffer::cleanup(VkDevice device)
{

}
