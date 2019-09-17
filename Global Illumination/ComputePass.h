#pragma once

#include "Vulkan.h"
#include "Image.h"
#include "Pipeline.h"
#include "RenderPass.h"

class ComputePass
{
public:
	void initialize(Vulkan* vk, VkExtent2D extent, VkExtent3D dispatchGroups, std::string computeShader, VkImageView inputImageView);
	void drawCall(Vulkan* vk);

	VkImageView getImageView() { return m_resultImage.getImageView(); }
	VkSemaphore getRenderFinishedSemaphore() { return m_renderCompleteSemaphore; }

	void setSemaphoreToWait(VkDevice device, std::vector<Semaphore> semaphores);
private:
	Image m_resultImage;
	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;
	VkDescriptorPool m_descriptorPool;
	Pipeline m_pipeline;

	std::vector<VkSemaphore> m_needToWaitSemaphores;
	std::vector<VkPipelineStageFlags> m_needToWaitStages;
	VkSemaphore m_renderCompleteSemaphore;
};

