#pragma once

#include <mutex>

#include "ModelPBR.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "GBuffer.h"
#include "Camera.h"
#include "ComputePass.h"

class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager();

    void load(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue computeQueue, VkSurfaceKHR surface, std::mutex * graphicsQueueMutex,
              std::vector<Image*> swapChainImages);

	void submit(VkDevice device, GLFWwindow* window, VkQueue graphicsQueue, VkQueue computeQueue, uint32_t swapChainImageIndex, Semaphore* imageAvailableSemaphore);

	void cleanup(VkDevice device);

    float getLoadingState() { return m_loadingState; }

private:
    float m_loadingState = 0.0f;

    CommandPool m_commandPool;
	DescriptorPool m_descriptorPool;

	Camera m_camera;

    ModelPBR m_model;

    GBuffer m_gbuffer;
	std::vector<Texture> m_gbufferTextures;

	ComputePass m_computePassFinalRender;
	Texture m_finalResultTexture;
};