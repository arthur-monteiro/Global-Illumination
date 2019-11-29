#pragma once

#include "RenderPass.h"

class GBuffer
{
public:
    GBuffer() = default;
    ~GBuffer();

    bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool commandPool, VkExtent2D extent);
    bool submit(VkDevice device, VkQueue graphicsQueue);
    void resize(VkDevice device, VkPhysicalDevice physicalDevice, int width, int height);

    void cleanup(VkDevice device);

private:
    RenderPass m_renderPass;
    std::vector<Attachment> m_attachments;
    std::vector<VkClearValue> m_clearValues;
};
