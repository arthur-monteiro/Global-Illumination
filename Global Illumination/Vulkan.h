#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <set>
#include <mutex>

#include "VulkanHelper.h"

class Vulkan
{
public:
	Vulkan() {}
	~Vulkan();

	void initialize(GLFWwindow* glfwWindowPointer);
	void cleanup();
	
// Getters
public:
	VkDevice getDevice() { return m_device; }
	VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }
	VkSurfaceKHR getSurface() { return m_surface; }

	VkQueue getGraphicsQueue() { return m_graphicsQueue; }
	VkQueue getPresentQueue() { return m_presentQueue; }
	VkQueue getComputeQueue() { return m_computeQueue; }
	std::mutex* getGraphicsQueueMutex() { return m_mutexGraphicsQueue; }
	std::mutex* getPresentQueueMutex() { return m_mutexPresentQueue; }
	std::mutex* getComputeQueueMutex() { return m_mutexComputeQueue; }

	bool isRayTracingAvailable() { return m_raytracingAvailable; }

private:
	/* Main Loading Functions */
	void createInstance();
	void setupDebugCallback();
	void pickPhysicalDevice();
	void createDevice();

private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix,
		const char* msg, void* userData)
	{
		std::cerr << "Validation Layer : " << msg << std::endl;

		return VK_FALSE;
	}

	static VkResult createDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pCallback);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

private:
	/* Vulkan attributes */
	VkInstance m_instance;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;

	/* Queues */
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	VkQueue m_computeQueue;

	/* Mutex queues */
	std::mutex* m_mutexGraphicsQueue;
	std::mutex* m_mutexPresentQueue;
	std::mutex* m_mutexComputeQueue;

	/* Extensions / Layers */
	std::vector<const char*> m_validationLayers = std::vector<const char*>();
	std::vector<const char*> m_deviceExtensions = std::vector<const char*>();
	VkDebugReportCallbackEXT m_debugCallback;

	/* Ray Tracing Availability */
	bool m_raytracingAvailable = false;
	std::vector<const char*> m_raytracingDeviceExtensions = std::vector<const char*>();
	VkPhysicalDeviceRayTracingPropertiesNV m_raytracingProperties = {};

	/* Properties */
	VkSampleCountFlagBits m_maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
};
