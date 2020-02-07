#include "SceneManager.h"

SceneManager::~SceneManager()
{

}

void SceneManager::load(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue computeQueue, VkSurfaceKHR surface,
        std::mutex * graphicsQueueMutex,  std::vector<Image*> swapChainImages)
{
	m_swapChainImages = swapChainImages;
	
	// Command Pool + Descriptor Pool
    m_graphicsCommandPool.initializeForGraphicsQueue(device, physicalDevice, surface);
	m_computeCommandPool.initializeForComputeQueue(device, physicalDevice, surface);
	m_descriptorPool.initialize(device);

	createResources(device, physicalDevice, graphicsQueue, graphicsQueueMutex);
	createUBOs(device, physicalDevice);
	
	// Camera
	m_camera.initialize(glm::vec3(1.4f, 1.2f, 0.3f), glm::vec3(2.0f, 0.9f, -0.3f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f, 5.0f,
		swapChainImages[0]->getExtent().width / (float)swapChainImages[0]->getExtent().height);

	createPasses(device, physicalDevice, graphicsQueue, graphicsQueueMutex, computeQueue, swapChainImages);

    m_loadingState = 1.0f;
}

void SceneManager::changeOption(std::string parameter, std::wstring value)
{
	if (parameter == "shadow")
	{
		if (value != L"No")
		{
			m_uboParamsData.drawShadows = 1;
			m_shadows.changeShadowType(value);
		}
		else
			m_uboParamsData.drawShadows = 0;

		m_needUpdateUBOParams = true;
	}
	else if (parameter == "msaa")
	{
		if (value == L"No")
			m_uboParamsData.sampleCount = 1;
		else if (value == L"2x")
			m_uboParamsData.sampleCount = 2;
		else if (value == L"4x")
			m_uboParamsData.sampleCount = 4;
		else if (value == L"8x")
			m_uboParamsData.sampleCount = 8;

		m_needUpdateUBOParams = true;
		m_needUpdateMSAA = true;
	}
	else if(parameter == "rtshadow_sample_count")
	{
		unsigned int sampleCount = 0;
		if (value == L"No")
			sampleCount = 1;
		else if (value == L"2x")
			sampleCount = 2;
		else if (value == L"4x")
			sampleCount = 4;
		else if (value == L"8x")
			sampleCount = 8;

		m_updateRTShadowSampleCount = sampleCount;
	}
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

	const auto currentTime = std::chrono::steady_clock::now();
	m_fpsCount++;
	int fpsModification = -1;
	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTimeFPSCounter).count() > 1000.0f)
	{
		fpsModification = m_fpsCount;

		m_fpsCount = 0;
		m_startTimeFPSCounter = currentTime;
	}

	if (m_needUpdateUBOParams)
	{
		m_uboParams.updateData(device, &m_uboParamsData);
		m_needUpdateUBOParams = false;
	}
	if(m_needUpdateMSAA)
	{
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
		if (m_uboParamsData.sampleCount == 2)
			sampleCount = VK_SAMPLE_COUNT_2_BIT;
		else if (m_uboParamsData.sampleCount == 4)
			sampleCount = VK_SAMPLE_COUNT_4_BIT;
		else if (m_uboParamsData.sampleCount == 8)
			sampleCount = VK_SAMPLE_COUNT_8_BIT;
		
		m_gbuffer.changeMSAA(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_swapChainImages[0]->getExtent(), sampleCount);
		recoverGetGBufferImages(device, physicalDevice, graphicsQueue, nullptr, m_swapChainImages);

		recreateFinalComputePass(device, physicalDevice, computeQueue, m_swapChainImages);

		m_needUpdateMSAA = false;
	}
	if(m_updateRTShadowSampleCount > 0)
	{
		m_shadows.changeRTSampleCount(device, m_updateRTShadowSampleCount);
		m_updateRTShadowSampleCount = 0;
	}
	
	m_camera.update(window);
    glm::mat4 mvp = m_camera.getProjection() * m_camera.getViewMatrix() * m_model.getTransformation();
	m_gbuffer.submit(device, graphicsQueue, mvp, m_model.getTransformation());

	if (m_uboParamsData.drawHUD == 1)
		m_HUD.submit(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, window, fpsModification, m_drawMenu);

	if(m_uboParamsData.drawShadows == 1)
		m_shadows.submit(device, graphicsQueue, {}, glm::inverse(m_camera.getViewMatrix()), glm::inverse(m_camera.getProjection()));

	m_uboLightingData.cameraPosition = glm::vec4(m_camera.getPosition(), 1.0f);
	m_uboLighting.updateData(device, &m_uboLightingData);

	std::vector<Semaphore*> semaphoreForEndPass = { m_gbuffer.getRenderCompleteSemaphore(), imageAvailableSemaphore };
	if (m_uboParamsData.drawShadows == 1)
		semaphoreForEndPass.push_back(m_shadows.getRenderCompleteSemaphore());
	if(m_uboParamsData.drawHUD == 1)
		semaphoreForEndPass.push_back(m_HUD.getRenderCompleteSemaphore());
	
	m_computePasses[swapChainImageIndex].submit(device, computeQueue, semaphoreForEndPass,
		m_computePassFinishedSemaphore.getSemaphore());
}

void SceneManager::resize(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue computeQueue, std::vector<Image*> swapChainImages)
{
	m_swapChainImages = swapChainImages;
	
	m_camera.setAspect(swapChainImages[0]->getExtent().width / (float)swapChainImages[0]->getExtent().height);

	/* -- HUD -- */
	createHUD(device, physicalDevice, graphicsQueue, nullptr, swapChainImages, true);

	/* -- GBuffer -- */
	m_gbuffer.resize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), swapChainImages[0]->getExtent());
	recoverGetGBufferImages(device, physicalDevice, graphicsQueue, nullptr, swapChainImages);

	/* -- Shadows -- */
	creatShadows(device, physicalDevice, graphicsQueue, nullptr, swapChainImages[0]->getExtent(), true);
	
	recreateFinalComputePass(device, physicalDevice, computeQueue, swapChainImages);
}

void SceneManager::cleanup(VkDevice device)
{
	m_model.cleanup(device);
	m_gbuffer.cleanup(device, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool());
	m_shadows.cleanup(device, m_graphicsCommandPool.getCommandPool());
	for (size_t i(0); i < m_computePasses.size(); ++i)
	{
		m_computePasses[i].cleanup(device, m_computeCommandPool.getCommandPool());
	}
	m_graphicsCommandPool.cleanup(device);
	m_descriptorPool.cleanup(device);
}

VkSemaphore SceneManager::getLastRenderFinishedSemaphore()
{
	return m_computePassFinishedSemaphore.getSemaphore();
}

void SceneManager::createResources(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex)
{
	// Model
	m_model.loadFromFile(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, graphicsQueueMutex,
		"Models/sponza/sponza.obj", "Models/sponza");
}

void SceneManager::createUBOs(VkDevice device, VkPhysicalDevice physicalDevice)
{
	m_uboLightingData.cameraPosition = glm::vec4(m_camera.getPosition(), 1.0f);
	m_uboLightingData.colorDirectionalLight = glm::vec4(10.0f);
	m_uboLightingData.directionDirectionalLight = glm::vec4(1.5f, -5.0f, -1.0f, 1.0f);
	m_uboLighting.initialize(device, physicalDevice, &m_uboLightingData, sizeof(m_uboLightingData));

	m_uboParamsData.drawHUD = 1;
	m_uboParamsData.drawShadows = 0;
	m_uboParams.initialize(device, physicalDevice, &m_uboParamsData, sizeof(m_uboParamsData));
}

void SceneManager::createPasses(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue,
	std::mutex* graphicsQueueMutex, VkQueue computeQueue, const std::vector<Image*>& swapChainImages)
{
	/* -- HUD -- */
	createHUD(device, physicalDevice, graphicsQueue, graphicsQueueMutex, swapChainImages, false);

	/* -- GBuffer -- */
	glm::mat4 mvp = m_camera.getProjection() * m_camera.getViewMatrix() * m_model.getTransformation();
	m_gbuffer.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), swapChainImages[0]->getExtent(), &m_model, mvp);
	recoverGetGBufferImages(device, physicalDevice, graphicsQueue, graphicsQueueMutex, swapChainImages);

	/* -- Shadows -- */
	creatShadows(device, physicalDevice, graphicsQueue, graphicsQueueMutex, swapChainImages[0]->getExtent(), false);

	/* -- Compute Pass -- */
	createFinalComputePass(device, physicalDevice, computeQueue, swapChainImages);
}

void SceneManager::createHUD(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue,
                             std::mutex* graphicsQueueMutex, const std::vector<Image*>& swapChainImages, bool recreate)
{
	if(!recreate)
	{
		graphicsQueueMutex->lock();
		m_HUD.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), graphicsQueue, swapChainImages[0]->getExtent(),
			changeOptionCallback, this);
		graphicsQueueMutex->unlock();
	}
	else
	{
		m_HUD.resize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), graphicsQueue, swapChainImages[0]->getExtent());
	}
	
	// Get images for HUD (should be 1)
	std::vector<Image*> HUDImages = m_HUD.getImages();
	m_HUDTextures.resize(HUDImages.size() - 1);
	for (size_t i(1); i < HUDImages.size(); ++i)
	{
		if(graphicsQueueMutex)
			graphicsQueueMutex->lock();
		HUDImages[i]->setImageLayout(device, m_graphicsCommandPool.getCommandPool(), graphicsQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		if(graphicsQueueMutex)
			graphicsQueueMutex->unlock();

		m_HUDTextures[i - static_cast<size_t>(1)].createFromImage(device, HUDImages[i]);
		m_HUDTextures[i - static_cast<size_t>(1)].createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);
	}
}

void SceneManager::recoverGetGBufferImages(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue,
	std::mutex* graphicsQueueMutex, std::vector<Image*> swapChainImages)
{
	// Get images from gbuffer
	std::vector<Image*> gbufferImages = m_gbuffer.getImages();
	m_gbufferTextures.resize(gbufferImages.size() - 1);
	for (size_t i(1); i < gbufferImages.size(); ++i)
	{
		if(graphicsQueueMutex)
			graphicsQueueMutex->lock();
		gbufferImages[i]->setImageLayout(device, m_graphicsCommandPool.getCommandPool(), graphicsQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		if(graphicsQueueMutex)
			graphicsQueueMutex->unlock();

		m_gbufferTextures[i - static_cast<size_t>(1)].createFromImage(device, gbufferImages[i]);
		m_gbufferTextures[i - static_cast<size_t>(1)].createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);
	}
}

void SceneManager::creatShadows(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue,
	std::mutex* graphicsQueueMutex, VkExtent2D extent, bool recreate)
{
	if (!recreate)
	{
		const glm::mat4 mvp = m_camera.getProjection() * m_camera.getViewMatrix() * m_model.getTransformation();
		if (graphicsQueueMutex)
			graphicsQueueMutex->lock();
		m_shadows.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, &m_model, mvp, extent);
		if (graphicsQueueMutex)
			graphicsQueueMutex->unlock();
	}
	else
		m_shadows.resize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, &m_model, extent);

	// Get result image
	Image* rtShadowImage = m_shadows.getImage();

	m_shadowsTexture.createFromImage(device, rtShadowImage);
	m_shadowsTexture.createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);
}

void SceneManager::createFinalComputePass(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue computeQueue,
	std::vector<Image*> swapChainImages)
{
	// Setting textures for compute pass
	std::vector<std::pair<Texture*, TextureLayout>> texturesForComputePass;

	// GBuffer
	{
		for (size_t i(0); i < m_gbufferTextures.size(); ++i)
		{
			TextureLayout textureLayout{};
			textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			textureLayout.binding = i;

			texturesForComputePass.emplace_back(&m_gbufferTextures[i], textureLayout);
		}
	}

	// RT Shadow
	{
		TextureLayout textureLayout{};
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = m_gbufferTextures.size();

		texturesForComputePass.emplace_back(&m_shadowsTexture, textureLayout);
	}

	// HUD
	{
		TextureLayout textureLayout{};
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = m_gbufferTextures.size() + 1; // GBuffer + RT Shadow

		texturesForComputePass.emplace_back(&m_HUDTextures[0], textureLayout);
	}

	// Swap chain transition (from present to general then from general to present)
	m_transitSwapchainToLayoutGeneral.resize(swapChainImages.size());
	for (size_t i(0); i < m_transitSwapchainToLayoutGeneral.size(); ++i)
	{
		m_transitSwapchainToLayoutGeneral[i].transitImageLayout(swapChainImages[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_GENERAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	m_transitSwapchainToLayoutPresent.resize(swapChainImages.size());
	for (size_t i(0); i < m_transitSwapchainToLayoutPresent.size(); ++i)
	{
		m_transitSwapchainToLayoutPresent[i].transitImageLayout(swapChainImages[i], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
	}

	// UBOs
	UniformBufferObjectLayout uboLightingLayout{};
	uboLightingLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboLightingLayout.binding = 7;

	UniformBufferObjectLayout uboParamsLayout{};
	uboParamsLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboParamsLayout.binding = 8;

	// Create compute passes
	m_computePasses.resize(swapChainImages.size());
	m_swapchainTextures.resize(swapChainImages.size());
	for (size_t i(0); i < m_computePasses.size(); ++i)
	{
		std::vector<std::pair<Texture*, TextureLayout>> textures = texturesForComputePass;

		TextureLayout textureLayout{};
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = textures.size();

		m_swapchainTextures[i].createFromImage(device, swapChainImages[i]);
		m_swapchainTextures[i].createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);

		textures.emplace_back(&m_swapchainTextures[i], textureLayout);

		std::string shaderPath = "Shaders/compute/comp.spv";
		if (m_uboParamsData.sampleCount > 1)
			shaderPath = "Shaders/compute/compMS.spv";

		m_computePasses[i].initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), swapChainImages[0]->getExtent(),
			{ 16, 16, 1 }, shaderPath, { { &m_uboLighting, uboLightingLayout}, { &m_uboParams, uboParamsLayout} },
			textures, { m_transitSwapchainToLayoutGeneral[i] }, { m_transitSwapchainToLayoutPresent[i] });
	}

	m_computePassFinishedSemaphore.initialize(device);
	m_computePassFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void SceneManager::recreateFinalComputePass(VkDevice device, VkPhysicalDevice physicalDevice,
	VkQueue computeQueue, std::vector<Image*> swapChainImages)
{
	for (size_t i(0); i < m_computePasses.size(); ++i)
	{
		m_computePasses[i].cleanup(device, m_computeCommandPool.getCommandPool());
	}
	createFinalComputePass(device, physicalDevice, computeQueue, swapChainImages);
}
