#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <array>
#include <functional>
#include <cstring>
#include <chrono>
#include <thread>  

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct MeshPipeline
{
	std::vector<VkBuffer> vertexBuffer;
	std::vector<VkBuffer> indexBuffer;
	std::vector<VkBuffer> instanceBuffer;
	std::vector<uint32_t> nbIndices;
	std::vector<VkDescriptorSet> descriptorSet; // autant de descriptorSet que de mesh
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	int frameBufferID = 0;

	void free(VkDevice device, VkDescriptorPool descriptorPool, bool recreate = false)
	{
		for (int i = 0; i < vertexBuffer.size(); ++i)
		{
			//vkDestroyBuffer(device, vertexBuffer[i], nullptr);
			//vkDestroyBuffer(device, indexBuffer[i], nullptr);
		}
		if (recreate)
		{
			//vkDestroyPipeline(device, pipeline, nullptr);
			//vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		}
		vkFreeDescriptorSets(device, descriptorPool, descriptorSet.size(), descriptorSet.data());
		vertexBuffer.clear();
		indexBuffer.clear();
		nbIndices.clear();
		descriptorSet.clear();
	}
};

struct FrameBuffer
{
	VkImageView imageView = VK_NULL_HANDLE;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkFramebuffer framebuffer;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	void free(VkDevice device)
	{
		if (imageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device, imageView, nullptr);
			vkDestroyImage(device, image, nullptr);
			vkFreeMemory(device, imageMemory, nullptr);
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);
	}
};

class Vulkan
{
public:
	void initialize(int width, int height, std::string appName, std::function<void(void*)> recreateCallback, void* instance, bool recreate);
	void createSwapchainFramebuffers(VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples, VkImageView colorImageView);
	void cleanup();

public:
	void SetValidationLayers(bool statut, std::vector<const char*> layers = std::vector<const char*>())
	{
		if (statut && !checkValidationLayerSupport(layers))
			throw std::runtime_error("Erreur : validation layer non disponible !");

		m_enableValidationLayers = statut;
		m_validationLayers = layers;
	}

	void SetDeviceExtensions(std::vector<const char*> extensions)
	{
		m_deviceExtensions = extensions;
	}

	VkFormat getSwapChainImageFormat() { return m_swapChainImageFormat; }
	VkDevice getDevice() { return m_device; }
	VkExtent2D getSwapChainExtend() { return m_swapChainExtent; }
	GLFWwindow* getWindow() { return m_window; }
	VkQueue getGraphicalQueue() { return m_graphicsQueue; }
	VkSemaphore* getRenderFinishedSemaphore() { return &m_renderFinishedSemaphore; }
	VkSampleCountFlagBits getMaxMsaaSamples() { return m_maxMsaaSamples; }

	void setRenderFinishedLastRenderPassSemaphore(VkSemaphore semaphore) { m_renderFinishedLastRenderPassSemaphore = semaphore; }

private:
	void createInstance();
	void setupDebugCallback();
	void pickPhysicalDevice();
	void createDevice();
	void createSwapChain();

	void cleanupSwapChain();

public :
	VkCommandPool createCommandPool();

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
		uint32_t arrayLayers, VkImageCreateFlags flags, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t arrayLayers);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	bool checkValidationLayerSupport(std::vector<const char*> layers);
	std::vector<const char*> getRequiredExtensions();
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType viewType);
	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t baseArrayLayer);
	void copyImage(VkImage source, VkImage dst, uint32_t width, uint32_t height, uint32_t baseArrayLayer, uint32_t mipLevel);
	FrameBuffer createFrameBuffer(VkExtent2D extent, VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples, VkImageView colorImageView, VkFormat depthFormat, VkFormat imageFormat);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t baseArrayLayer);
	VkSampleCountFlagBits getMaxUsableSampleCount();

	void fillCommandBuffer(VkRenderPass renderPass, std::vector<MeshPipeline> meshes);
	void createSemaphores();

	void drawFrame();

	void recreateSwapChain();

private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix,
		const char* msg, void* userData)
	{
		std::cerr << "Validation Layer : " << msg << std::endl;

		return VK_FALSE;
	}

	static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pCallback);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void onWindowResized(GLFWwindow* window, int width, int height)
	{
		if (width == 0 || height == 0) return;

		Vulkan* app = reinterpret_cast<Vulkan*>(glfwGetWindowUserPointer(window));
		app->recreateSwapChain();
	}

private:
	std::function<void(void*)> m_recreateCallback;
	void* m_systemInstance;

	bool m_enableValidationLayers = false;
	std::vector<const char*> m_validationLayers = std::vector<const char*>();
	std::vector<const char*> m_deviceExtensions = std::vector<const char*>();
	VkDebugReportCallbackEXT m_callback;

protected:
	GLFWwindow* m_window;

	VkInstance m_instance;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkSwapchainKHR m_swapChain;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImageView> m_swapChainImageViews;
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;
	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;

	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffersSwapChain;

	VkSemaphore m_imageAvailableSemaphore;
	VkSemaphore m_renderFinishedSemaphore;

	VkSemaphore m_renderFinishedLastRenderPassSemaphore = VK_NULL_HANDLE;

	VkSampleCountFlagBits m_maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	
	int m_maxFPS = 60;
	std::chrono::high_resolution_clock::time_point m_lastFrameTimeCounter = std::chrono::high_resolution_clock::now();
};
