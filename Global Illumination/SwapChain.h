#pragma once

#include "VulkanHelper.h"
#include "Image.h"
#include "Semaphore.h"

#include <iostream>

class SwapChain
{
public:
	SwapChain() {}
	~SwapChain();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow* window);

	uint32_t getCurrentImage(VkDevice device);
	void present(VkQueue presentQueue, VkSemaphore waitSemaphore, uint32_t imageIndex);
	void recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow* window);

	bool cleanup(VkDevice device);

// Getters
public:
    std::vector<Image*> getImages();
    Semaphore * getImageAvailableSemaphore() { return &m_imageAvailableSemaphore; }

private:
	VkSwapchainKHR m_swapChain;
	std::vector<Image> m_images;

	Semaphore m_imageAvailableSemaphore;

private:
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
};