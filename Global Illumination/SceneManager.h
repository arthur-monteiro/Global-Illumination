#pragma once

#include <mutex>

#include "ModelPBR.h"
#include "Model2DTextured.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "GBuffer.h"
#include "Camera.h"
#include "ComputePass.h"
#include "Command.h"
#include "Operation.h"
#include "HUD.h"
#include "Shadows.h"

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
	void createPasses(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex, VkQueue computeQueue,
	                  const std::vector<Image*>& swapChainImages);

	// Modules creation
	void createHUD(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex,
	               const std::vector<Image*>& swapChainImages, bool recreate);
	void recoverGetGBufferImages(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex, std::vector<Image*> swapChainImages);
	void creatShadows(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex, VkExtent2D extent, bool recreate);
	void createFinalComputePass(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue computeQueue, std::vector<Image*> swapChainImages);

	void recreateFinalComputePass(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue computeQueue, std::vector<Image*> swapChainImages);

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

    GBuffer m_gbuffer;
	std::vector<Texture> m_gbufferTextures;

	HUD m_HUD;
	std::vector<Texture> m_HUDTextures;
	bool m_drawMenu = false;

	Shadows m_shadows;
	Texture m_shadowsTexture;

	std::vector<ComputePass> m_computePasses;
	Semaphore m_computePassFinishedSemaphore;
	std::vector<Texture> m_swapchainTextures;
	std::vector<Operation> m_transitSwapchainToLayoutGeneral;
	std::vector<Operation> m_transitSwapchainToLayoutPresent;

	struct LightingUBO
    {
        glm::vec4 cameraPosition;

        glm::vec4 directionDirectionalLight;
        glm::vec4 colorDirectionalLight;
    };
	UniformBufferObject m_uboLighting;
	LightingUBO m_uboLightingData;

	// Parameters
    struct ParamsUBO
    {
		glm::uint drawHUD;
		glm::uint drawShadows;
		glm::uint sampleCount = 1;
    };
	UniformBufferObject m_uboParams;
	ParamsUBO m_uboParamsData;
	bool m_needUpdateUBOParams = false;
	bool m_needUpdateMSAA = false;
	unsigned int m_updateRTShadowSampleCount = 0;

	int m_oldEscapeState = GLFW_RELEASE;
};