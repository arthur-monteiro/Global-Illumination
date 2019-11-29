#pragma once

#include <chrono>
#include <math.h>   

#include "RenderPass.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "Model2DTextured.h"
#include "Renderer.h"
#include "UniformBufferObject.h"

class LoadingManager
{
public:
	LoadingManager() {}
	~LoadingManager();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, std::vector<Image*> swapChainImages);
	bool submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, const Semaphore& imageAvailableSemaphore);
	void resize(VkDevice device, VkPhysicalDevice physicalDevice, std::vector<Image*> swapChainImages);

	void cleanup(VkDevice device);

// Getters
public:
    VkSemaphore getLastRenderFinishedSemaphore();

private:
	/* Render Pass */
    RenderPass m_mainRenderPass;
	std::vector<Attachment> m_attachments;
	std::vector<VkClearValue> m_clearValues;

	CommandPool m_uniqueCommandPool;
	DescriptorPool m_uniqueDescriptorPool;

	Model2DTextured m_fullScreenQuad;
	Renderer m_fullScreenQuadRenderer;
	Texture m_fullScreenLoadingTexture;

	Model2DTextured m_loadingLogoQuad;
	Renderer m_loadingLogoQuadRenderer;
	Texture m_loadingLogoTexture;
	float m_logoWidth = 1.0f;
	float m_logoOpacity = 1.0f;
	UniformBufferObject m_uboLogoOpacity;
	UniformBufferObject m_uboLogoOffset;
	std::chrono::high_resolution_clock::time_point m_timerStart = std::chrono::high_resolution_clock::now();
	int m_loadingLogoLoopMillisecond = 600;
};

