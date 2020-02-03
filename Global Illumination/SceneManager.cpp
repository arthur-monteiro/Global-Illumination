#include "SceneManager.h"

SceneManager::~SceneManager()
{

}

void SceneManager::load(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue computeQueue, VkSurfaceKHR surface,
        std::mutex * graphicsQueueMutex,  std::vector<Image*> swapChainImages)
{
	// Command Pool + Descriptor Pool
    m_graphicsCommandPool.initializeForGraphicsQueue(device, physicalDevice, surface);
	m_computeCommandPool.initializeForComputeQueue(device, physicalDevice, surface);
	m_descriptorPool.initialize(device);

	// Camera
	m_camera.initialize(glm::vec3(1.4f, 1.2f, 0.3f), glm::vec3(2.0f, 0.9f, -0.3f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f, 5.0f, 
		swapChainImages[0]->getExtent().width / (float)swapChainImages[0]->getExtent().height);

	// Font
	graphicsQueueMutex->lock();
	m_HUD.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), graphicsQueue, swapChainImages[0]->getExtent());
	graphicsQueueMutex->unlock();

	// Model
    m_model.loadFromFile(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, graphicsQueueMutex,
            "Models/sponza/sponza.obj", "Models/sponza");

	// GBuffer
	glm::mat4 mvp = m_camera.getProjection() * m_camera.getViewMatrix() * m_model.getTransformation();
    m_gbuffer.initialize(device, physicalDevice, surface, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), swapChainImages[0]->getExtent(), &m_model, mvp);

	// Get images from gbuffer
	std::vector<Image*> gbufferImages = m_gbuffer.getImages();
	m_gbufferTextures.resize(gbufferImages.size() - 1);
	for (size_t i(1); i < gbufferImages.size(); ++i)
	{
		graphicsQueueMutex->lock();
		gbufferImages[i]->setImageLayout(device, m_graphicsCommandPool.getCommandPool(), graphicsQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		graphicsQueueMutex->unlock();

		m_gbufferTextures[i - static_cast<size_t>(1)].createFromImage(device, gbufferImages[i]);
		m_gbufferTextures[i - static_cast<size_t>(1)].createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);
	}

	// RT Shadows
	graphicsQueueMutex->lock();
	m_rtShadowPass.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, &m_model, mvp);
	graphicsQueueMutex->unlock();

	m_rtShadowPassFinishedSemaphore.initialize(device);
	m_rtShadowPassFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	// Get images from gbuffer
	Image* rtShadowImage = m_rtShadowPass.getImage();

	m_rtShadowTexture.createFromImage(device, rtShadowImage);
	m_rtShadowTexture.createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);

	// Setting textures for compute pass
	std::vector<std::pair<Texture*, TextureLayout>> texturesForComputePass;
	for (int i(1); i < gbufferImages.size(); ++i)
	{
		TextureLayout textureLayout;
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = i - 1;

		texturesForComputePass.push_back({ &m_gbufferTextures[i - 1], textureLayout });
	}
	
	TextureLayout textureLayout;
	textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureLayout.binding = gbufferImages.size() - 1;
	
	texturesForComputePass.push_back({ &m_rtShadowTexture, textureLayout });

	std::vector<Image*> HUDImages = m_HUD.getImages();
	m_HUDTextures.resize(HUDImages.size() - 1);
	for (size_t i(1); i < HUDImages.size(); ++i)
	{
		graphicsQueueMutex->lock();
		HUDImages[i]->setImageLayout(device, m_graphicsCommandPool.getCommandPool(), graphicsQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		graphicsQueueMutex->unlock();

		m_HUDTextures[i - static_cast<size_t>(1)].createFromImage(device, HUDImages[i]);
		m_HUDTextures[i - static_cast<size_t>(1)].createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);
	}

	textureLayout.binding = gbufferImages.size();

	texturesForComputePass.push_back({ &m_HUDTextures[0], textureLayout });

	m_transitSwapchainToLayoutGeneral.resize(swapChainImages.size());
	for (int i(0); i < m_transitSwapchainToLayoutGeneral.size(); ++i)
	{
		m_transitSwapchainToLayoutGeneral[i].transitImageLayout(swapChainImages[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_GENERAL, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	m_transitSwapchainToLayoutPresent.resize(swapChainImages.size());
	for (int i(0); i < m_transitSwapchainToLayoutPresent.size(); ++i)
	{
		m_transitSwapchainToLayoutPresent[i].transitImageLayout(swapChainImages[i], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
	}

	// UBO
	UniformBufferObjectLayout uboLightingLayout;
	uboLightingLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboLightingLayout.binding = 7;

	m_uboLightingData.cameraPosition = glm::vec4(m_camera.getPosition(), 1.0f);
	m_uboLightingData.colorDirectionalLight = glm::vec4(10.0f);
	m_uboLightingData.directionDirectionalLight = glm::vec4(1.5f, -5.0f, -1.0f, 1.0f);
	m_uboLighting.initialize(device, physicalDevice, &m_uboLightingData, sizeof(m_uboLightingData));

	UniformBufferObjectLayout uboParamsLayout;
	uboParamsLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboParamsLayout.binding = 8;

	m_uboParamsData.drawHUD = 1;
	m_uboParams.initialize(device, physicalDevice, &m_uboParamsData, sizeof(m_uboParamsData));

	m_computePasses.resize(swapChainImages.size()); 
	m_swapchainTextures.resize(swapChainImages.size());
	for (int i(0); i < m_computePasses.size(); ++i)
	{
		std::vector<std::pair<Texture*, TextureLayout>> textures = texturesForComputePass;

		TextureLayout textureLayout;
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = textures.size();

		m_swapchainTextures[i].createFromImage(device, swapChainImages[i]);
		m_swapchainTextures[i].createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);

		textures.emplace_back(&m_swapchainTextures[i], textureLayout);

		m_computePasses[i].initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), swapChainImages[0]->getExtent(),
			{ 16, 16, 1 }, "Shaders/compute/comp.spv", { { &m_uboLighting, uboLightingLayout}, { &m_uboParams, uboParamsLayout} }, 
			textures, { m_transitSwapchainToLayoutGeneral[i] }, { m_transitSwapchainToLayoutPresent[i] });
	}

	m_computePassFinishedSemaphore.initialize(device);
	m_computePassFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    m_loadingState = 1.0f;
}

void SceneManager::submit(VkDevice device, VkPhysicalDevice physicalDevice, GLFWwindow* window, VkQueue graphicsQueue, VkQueue computeQueue, uint32_t swapChainImageIndex, Semaphore * imageAvailableSemaphore)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && m_oldEscapeState == GLFW_RELEASE)
	{
		m_drawMenu = m_drawMenu ? false : true;
		m_camera.setFixed(m_drawMenu);
	}
	m_oldEscapeState = glfwGetKey(window, GLFW_KEY_ESCAPE);
	
	static auto startTime = std::chrono::steady_clock::now();

	auto currentTime = std::chrono::steady_clock::now();
	m_fpsCount++;
	int fpsModification = -1;
	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTimeFPSCounter).count() > 1000.0f)
	{
		fpsModification = m_fpsCount;

		m_fpsCount = 0;
		m_startTimeFPSCounter = currentTime;
	}
	
	m_camera.update(window);
    glm::mat4 mvp = m_camera.getProjection() * m_camera.getViewMatrix() * m_model.getTransformation();
	m_gbuffer.submit(device, graphicsQueue, mvp, m_model.getTransformation());

	m_HUD.submit(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, fpsModification, m_drawMenu);

	m_rtShadowPass.submit(device, graphicsQueue, {}, m_rtShadowPassFinishedSemaphore.getSemaphore(), glm::inverse(m_camera.getViewMatrix()), glm::inverse(m_camera.getProjection()));

	m_uboLightingData.cameraPosition = glm::vec4(m_camera.getPosition(), 1.0f);
	m_uboLighting.updateData(device, &m_uboLightingData);
	m_computePasses[swapChainImageIndex].submit(device, computeQueue, { m_gbuffer.getRenderCompleteSemaphore(), imageAvailableSemaphore, &m_rtShadowPassFinishedSemaphore, m_HUD.getRenderCompleteSemaphore() },
		m_computePassFinishedSemaphore.getSemaphore());
}

void SceneManager::cleanup(VkDevice device)
{
	m_model.cleanup(device);
	m_gbuffer.cleanup(device, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool());
	m_rtShadowPass.cleanup(device, m_graphicsCommandPool.getCommandPool());
	m_graphicsCommandPool.cleanup(device);
	m_descriptorPool.cleanup(device);
}

VkSemaphore SceneManager::getLastRenderFinishedSemaphore()
{
	return m_computePassFinishedSemaphore.getSemaphore();
}
