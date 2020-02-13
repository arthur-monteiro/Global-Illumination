#pragma once

#include <mutex>

#include "ModelPBR.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "GBuffer.h"
#include "Camera.h"
#include "ComputePass.h"
#include "Operation.h"
#include "HUD.h"
#include "Shadows.h"
#include "DepthPass.h"
#include "PostProcessAA.h"
#include "PBRCompute.h"
#include "AmbientOcclusion.h"

class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager();

    void load(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue computeQueue, VkSurfaceKHR surface, std::mutex * graphicsQueueMutex,
              std::vector<Image*> swapChainImages);

	void changeOption(std::string parameter, std::wstring value);

	void submit(VkDevice device, VkPhysicalDevice physicalDevice, GLFWwindow* window, VkQueue graphicsQueue, VkQueue computeQueue, uint32_t swapChainImageIndex, Semaphore* imageAvailableSemaphore);
	void resize(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue computeQueue, std::vector<Image*> swapChainImages);

	void cleanup(VkDevice device);

    float getLoadingState() const { return m_loadingState; }
    VkSemaphore getLastRenderFinishedSemaphore();

	static void changeOptionCallback(void* instance, std::string parameter, std::wstring value) { reinterpret_cast<SceneManager*>(instance)->changeOption(parameter, value); }

private:
	void createResources(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex);
	void createUBOs(VkDevice device, VkPhysicalDevice physicalDevice);
	void createSwapChainTextures(VkDevice device, const std::vector<Image*>& swapChainImages);
	void createPasses(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex, VkQueue computeQueue,
	                  const std::vector<Image*>& swapChainImages);

	// Modules creation
	void createHUD(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex,
	               const std::vector<Image*>& swapChainImages, bool recreate);
	void recoverGBufferImages(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex);
	void createPBRComputePass(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue computeQueue, bool recreate);
	void createPostProcessAAComputePass(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue computeQueue, bool recreate);

	std::vector<Texture*> getSwapChainTextures();

private:
    float m_loadingState = 0.0f;
	std::vector<Image*> m_swapChainImages;

	int m_fpsCount = 0;
	std::chrono::steady_clock::time_point m_startTimeFPSCounter = std::chrono::steady_clock::now();

    CommandPool m_graphicsCommandPool;
	CommandPool m_computeCommandPool;
	DescriptorPool m_descriptorPool;

	Camera m_camera;

    ModelPBR m_model;

	DepthPass m_preDepthPass;
	Texture m_depthPassTexture;
	PostProcessAA m_postProcessAA;
	bool m_postProcessAACreated = false;
	bool m_usePostProcessAA = false;
	bool m_needUpdatePostProcessAA = false;
	VkSampleCountFlagBits m_postProcessAASampleCount = VK_SAMPLE_COUNT_1_BIT;

    GBuffer m_gbuffer;
	std::vector<Texture> m_gbufferTextures;
	Texture m_gbufferDepthTexture;
	bool m_needUpdateUpscale = false;

	HUD m_HUD;
	std::vector<Texture> m_HUDTextures;
	bool m_drawMenu = false;

	Shadows m_shadows;
	unsigned int m_updateRTShadowSampleCount = 0;
	std::wstring m_shadowAlgorithm = L"";

	AmbientOcclusion m_ambientOcclusion;
	int m_updateSSAOPower = 0;
	std::wstring m_aoAlgorithm = L"";

	PBRCompute m_pbrCompute;
	ParamsUBO m_uboParamsData;
	bool m_needUpdateUBOParams = false;
	
	std::vector<Texture> m_swapchainTextures;
	std::vector<Operation> m_transitSwapchainToLayoutGeneral;
	std::vector<Operation> m_transitSwapchainToLayoutPresent;

	int m_oldEscapeState = GLFW_RELEASE;
};