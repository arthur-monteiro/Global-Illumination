#pragma once

#include "VulkanHelper.h"
#include "Image.h"

#include <iostream>

class SwapChain
{
public:
	SwapChain() {}
	~SwapChain();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow* window);
	bool cleanup(VkDevice device);

private:
	VkSwapchainKHR m_swapChain;
	std::vector<Image> m_images;

private:
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
};