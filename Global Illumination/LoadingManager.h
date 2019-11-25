#pragma once

#include "RenderPass.h"
#include "CommandPool.h"
#include "Model2D.h"
#include "Renderer.h"

class LoadingManager
{
public:
	LoadingManager() {}
	~LoadingManager();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, std::vector<Image*> swapChainImages);
	bool submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, const Semaphore& imageAvailableSemaphore);

	void cleanup(VkDevice device);

// Getters
public:
    VkSemaphore getLastRenderFinishedSemaphore();

private:
    RenderPass m_mainRenderPass;
	CommandPool m_uniqueCommandPool;
	Model2D m_quad;
	Renderer m_quadRenderer;
};

