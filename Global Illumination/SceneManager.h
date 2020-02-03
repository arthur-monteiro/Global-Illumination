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
#include "RayTracingPass.h"
#include "HUD.h"

class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager();

    void load(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue computeQueue, VkSurfaceKHR surface, std::mutex * graphicsQueueMutex,
              std::vector<Image*> swapChainImages);

	void submit(VkDevice device, VkPhysicalDevice physicalDevice, GLFWwindow* window, VkQueue graphicsQueue, VkQueue computeQueue, uint32_t swapChainImageIndex, Semaphore* imageAvailableSemaphore);

	void cleanup(VkDevice device);

    float getLoadingState() { return m_loadingState; }
    VkSemaphore getLastRenderFinishedSemaphore();

private:
    float m_loadingState = 0.0f;

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

	RayTracingPass m_rtShadowPass;
	Texture m_rtShadowTexture;
	Semaphore m_rtShadowPassFinishedSemaphore;

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

    struct ParamsUBO
    {
		glm::uint drawHUD;
    };
	UniformBufferObject m_uboParams;
	ParamsUBO m_uboParamsData;

	int m_oldEscapeState = GLFW_RELEASE;
};