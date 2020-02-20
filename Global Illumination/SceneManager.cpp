#include "SceneManager.h"

SceneManager::~SceneManager()
{
}

void SceneManager::load(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue computeQueue, VkSurfaceKHR surface,
        std::mutex * graphicsQueueMutex,  std::vector<Image*> swapChainImages, HardwareCapabilities hardwareCapabilities)
{
	m_swapChainImages = swapChainImages;
	m_hardwareCapabilities = hardwareCapabilities;
	
	// Command Pool + Descriptor Pool
    m_graphicsCommandPool.initializeForGraphicsQueue(device, physicalDevice, surface);
	m_computeCommandPool.initializeForComputeQueue(device, physicalDevice, surface);
	m_descriptorPool.initialize(device);

	createResources(device, physicalDevice, graphicsQueue, graphicsQueueMutex);
	createUBOs(device, physicalDevice);
	
	// Camera
	m_camera.initialize(glm::vec3(1.4f, 1.2f, 0.3f), glm::vec3(2.0f, 0.9f, -0.3f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f, 5.0f,
		swapChainImages[0]->getExtent().width / static_cast<float>(swapChainImages[0]->getExtent().height));

	createSwapChainTextures(device, swapChainImages);
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
			m_shadowAlgorithm = value;
		}
		else
		{
			m_shadowAlgorithm = L"";
			m_uboParamsData.drawShadows = 0;
		}

		m_needUpdateUBOParams = true;
	}
	else if (parameter == "upscale")
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
		m_needUpdateUpscale = true;
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
	else if(parameter == "msaa")
	{
		if (value == L"No")
			m_postProcessAASampleCount = VK_SAMPLE_COUNT_1_BIT;
		else if (value == L"2x")
			m_postProcessAASampleCount = VK_SAMPLE_COUNT_2_BIT;
		else if (value == L"4x")
			m_postProcessAASampleCount = VK_SAMPLE_COUNT_4_BIT;
		else if (value == L"8x")
			m_postProcessAASampleCount = VK_SAMPLE_COUNT_8_BIT;

		if (m_postProcessAASampleCount != VK_SAMPLE_COUNT_1_BIT)
			m_usePostProcessAA = true;
		else
			m_usePostProcessAA = false;

		m_needUpdatePostProcessAA = true;
	}
	else if(parameter == "ao")
	{
		if (value != L"No")
		{
			m_uboParamsData.useAO = 1;
			m_aoAlgorithm = value;
		}
		else
		{
			m_uboParamsData.useAO = 0;
			m_aoAlgorithm = L"";
		}

		m_needUpdateUBOParams = true;
	}
	else if (parameter == "ssao_power")
	{
		int power = 1;
		if (value == L"2")
			power = 2;
		else if (value == L"5")
			power = 5;
		else if (value == L"10")
			power = 10;
		else if (value == L"100")
			power = 100;

		m_updateSSAOPower = power;
	}
	else if (parameter == "bloom")
	{
		if (value == L"true")
			m_useBloom = true;
		else
			m_useBloom = false;

		m_bloomChange = true;
	}
	else if (parameter == "reflection")
	{
		if (value == L"SSR")
			m_useReflections = true;
		else
			m_useReflections = false;

		m_reflectionAlgorithm = value;
	}
}

void SceneManager::submit(VkDevice device, VkPhysicalDevice physicalDevice, GLFWwindow* window, VkQueue graphicsQueue, VkQueue computeQueue, uint32_t swapChainImageIndex, Semaphore * imageAvailableSemaphore)
{
	/* -- Camera update -- */
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && m_oldEscapeState == GLFW_RELEASE)
	{
		m_drawMenu = m_drawMenu ? false : true;
		m_camera.setFixed(m_drawMenu);
	}
	m_oldEscapeState = glfwGetKey(window, GLFW_KEY_ESCAPE);
	m_camera.update(window);
	glm::mat4 mvp = m_camera.getProjection() * m_camera.getViewMatrix() * m_model.getTransformation();
	
	/* -- FPS Counter -- */
	static auto startTime = std::chrono::steady_clock::now();

	const auto currentTime = std::chrono::steady_clock::now();
	m_fpsCount++;
	int fpsModification = -1;
	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTimeFPSCounter).count() > 1000.0f)
	{
		std::cout << (1.0 / m_fpsCount) * 1000.0 << " ms" << std::endl;
		fpsModification = m_fpsCount;

		m_fpsCount = 0;
		m_startTimeFPSCounter = currentTime;
	}

	bool gbufferChanged = false; // indicated if gbuffer image have changed
	bool aoChanged = false;
	bool shadowsChanged = false;
	bool reflectionChanged = false;
	bool bloomChanged = false;
	
	// Update UBO params
	if (m_needUpdateUBOParams)
	{
		m_pbrCompute.updateParameters(device, m_uboParamsData);
		m_needUpdateUBOParams = false;
	}
	// Upscale
	if(m_needUpdateUpscale)
	{
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
		if (m_uboParamsData.sampleCount == 2)
			sampleCount = VK_SAMPLE_COUNT_2_BIT;
		else if (m_uboParamsData.sampleCount == 4)
			sampleCount = VK_SAMPLE_COUNT_4_BIT;
		else if (m_uboParamsData.sampleCount == 8)
			sampleCount = VK_SAMPLE_COUNT_8_BIT;
		
		m_gbuffer.changeSampleCount(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_swapChainImages[0]->getExtent(), sampleCount);
		recoverGBufferImages(device, physicalDevice, graphicsQueue, nullptr);

		gbufferChanged = true;
		m_needUpdateUpscale = false;
	}
	else if (m_needUpdatePostProcessAA) // can't update gbuffer sample count and post process aa
	{
		if (m_postProcessAASampleCount == VK_SAMPLE_COUNT_1_BIT)
		{
			m_preDepthPass.cleanup(device, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool());
			m_gbuffer.useDepthAsStorage(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_swapChainImages[0]->getExtent(), false);
		}
		else
		{
			if(m_postProcessAASampleCount != VK_SAMPLE_COUNT_2_BIT)
				m_preDepthPass.cleanup(device, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool());

			m_preDepthPass.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), m_swapChainImages[0]->getExtent(), 
				m_postProcessAASampleCount, &m_model, mvp, true);
			m_depthPassTexture.createFromImage(device, m_preDepthPass.getImage());
			m_gbuffer.useDepthAsStorage(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_swapChainImages[0]->getExtent(), true);
		}
		recoverGBufferImages(device, physicalDevice, graphicsQueue, nullptr);
		gbufferChanged = true;
		m_needUpdatePostProcessAA = false;
	}
	// RT Shadows UBO
	if(m_updateRTShadowSampleCount > 0)
	{
		m_shadows.changeRTSampleCount(device, m_updateRTShadowSampleCount);
		m_updateRTShadowSampleCount = 0;
	}
	// SSAO UBO
	if (m_updateSSAOPower > 0)
	{
		m_ambientOcclusion.updateSSAOPower(device, m_updateSSAOPower);
		m_updateSSAOPower = 0;
	}

	// Create / delete passes
	if (m_ambientOcclusion.setAlgorithm(device, physicalDevice, m_computeCommandPool.getCommandPool(), computeQueue, m_descriptorPool.getDescriptorPool(),
		m_camera.getProjection(), &m_gbufferTextures[0], &m_gbufferTextures[2], m_aoAlgorithm))
		aoChanged = true;

	if (m_shadows.changeShadowType(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), graphicsQueue, &m_model,
		mvp, glm::vec4(1.5f, -5.0f, -1.0f, 1.0f), m_camera.getNear(), m_camera.getFar(), m_swapChainImages[0]->getExtent(), m_shadowAlgorithm))
		shadowsChanged = true;

	if (m_reflections.setAlgorithm(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), computeQueue, m_toneMapping.getOutputTexture(),
		&m_gbufferTextures[0], &m_gbufferTextures[2], m_camera.getProjection(), static_cast<VkSampleCountFlagBits>(m_uboParamsData.sampleCount), m_reflectionAlgorithm))
		reflectionChanged = true;

	if (m_bloomChange)
	{
		if (m_useBloom)
			m_bloom.initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), computeQueue, m_pbrCompute.getTextureOutput());
		else
			m_bloom.cleanup(device, m_computeCommandPool.getCommandPool());

		m_bloomChange = false;
		bloomChanged = true;
	}

	// Recreate AO
	if(gbufferChanged)
	{
		if (m_uboParamsData.useAO == 1 && m_ambientOcclusion.isReady())
			m_ambientOcclusion.recreate(device, physicalDevice, m_computeCommandPool.getCommandPool(), computeQueue, m_descriptorPool.getDescriptorPool(),
				m_camera.getProjection(), &m_gbufferTextures[0], &m_gbufferTextures[2]);
	}

	// Recreate PBR
	bool pbrChanged = false;
	if(aoChanged || shadowsChanged || gbufferChanged)
	{
		createPBRComputePass(device, physicalDevice, computeQueue, true);
		pbrChanged = true;
	}

	// Recreate bloom
	if(pbrChanged)
	{
		if (m_useBloom)
		{
			m_bloom.cleanup(device, m_computeCommandPool.getCommandPool());
			m_bloom.initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), computeQueue, m_pbrCompute.getTextureOutput());
		}
	}

	// Recreate tone mapping
	if(pbrChanged || bloomChanged)
	{
		m_toneMapping.recreate(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), computeQueue,
			m_useBloom ? m_bloom.getOutputTexture() : m_pbrCompute.getTextureOutput());
	}

	// Recreate reflections
	if(pbrChanged || bloomChanged) // tone mapping changed
	{
		m_reflections.recreate(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), computeQueue, m_toneMapping.getOutputTexture(),
			&m_gbufferTextures[0], &m_gbufferTextures[2], m_camera.getProjection(), static_cast<VkSampleCountFlagBits>(m_uboParamsData.sampleCount));
	}

	// Recreate post process AA
	if(pbrChanged || bloomChanged || reflectionChanged)
	{
		if (m_postProcessAASampleCount != VK_SAMPLE_COUNT_1_BIT)
			createPostProcessAAComputePass(device, physicalDevice, computeQueue, true);
		else if (m_postProcessAACreated)
		{
			m_postProcessAACreated = false;
			m_postProcessAA.cleanup(device, m_computeCommandPool.getCommandPool());
		}
	}

	// Recreate merge
	if (pbrChanged || bloomChanged || reflectionChanged)
	{
		std::vector<Texture*> swapChainTextures = getSwapChainTextures();
		m_merge.recreate(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), 
			m_usePostProcessAA ? m_postProcessAA.getOutputTexture() : m_useReflections ? m_reflections.getTextureOutput() : m_toneMapping.getOutputTexture(), &m_HUDTextures[0],
			swapChainTextures, m_transitSwapchainToLayoutGeneral, m_transitSwapchainToLayoutPresent);
	}

	/* -- Submit -- */
	m_gbuffer.submit(device, graphicsQueue, m_camera.getViewMatrix(), m_model.getTransformation(), m_camera.getProjection());
	m_skybox.submit(device, graphicsQueue, m_camera.getViewMatrix(), m_camera.getProjection());

	m_HUD.submit(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, window, fpsModification, m_drawMenu);

	if (m_uboParamsData.drawShadows == 1 && m_shadows.isReady())
		m_shadows.submit(device, graphicsQueue, {}, glm::inverse(m_camera.getViewMatrix()), glm::inverse(m_camera.getProjection()), m_camera.getViewMatrix(),
			m_model.getTransformation(), m_camera.getProjection(), m_camera.getNear(), m_camera.getFOV(), glm::vec3(1.5f, -5.0f, -1.0f), m_camera.getPosition(),
			m_camera.getOrientation());

	if(m_uboParamsData.useAO == 1 && m_ambientOcclusion.isReady())
		m_ambientOcclusion.submit(device, computeQueue, { m_gbuffer.getRenderCompleteSemaphore() });

	std::vector<Semaphore*> semaphoresToWaitPBR = { m_skybox.getRenderCompleteSemaphore() };
	if (m_uboParamsData.drawShadows == 1 && m_shadows.isReady())
		semaphoresToWaitPBR.push_back(m_shadows.getRenderCompleteSemaphore());
	if (m_uboParamsData.useAO == 1 && m_ambientOcclusion.isReady())
		semaphoresToWaitPBR.push_back(m_ambientOcclusion.getSemaphore());
	else // ambient occlusion waits for gBuffer
		semaphoresToWaitPBR.push_back(m_gbuffer.getRenderCompleteSemaphore());

	m_pbrCompute.submit(device, computeQueue, semaphoresToWaitPBR, glm::transpose(glm::inverse(m_camera.getViewMatrix())) * glm::vec4(1.5f, -5.0f, -1.0f, 1.0f));
	if (m_useBloom && m_bloom.isReady())
		m_bloom.submit(device, computeQueue, { m_pbrCompute.getSemaphore() });
	m_toneMapping.submit(device, computeQueue, { (m_useBloom && m_bloom.isReady()) ? m_bloom.getSemaphore() : m_pbrCompute.getSemaphore() });
	if (m_useReflections && m_reflections.isReady())
		m_reflections.submit(device, computeQueue, { m_toneMapping.getSemaphore() });

	std::vector<Semaphore*> semaphoresToWaitMerge = { imageAvailableSemaphore, m_HUD.getRenderCompleteSemaphore() };

	if (m_usePostProcessAA && m_postProcessAACreated)
	{
		m_preDepthPass.submit(device, graphicsQueue, mvp);
		m_postProcessAA.submit(device, computeQueue, { m_useReflections && m_reflections.isReady() ? m_reflections.getSemaphore() : m_toneMapping.getSemaphore(), 
			m_preDepthPass.getSemaphore() });

		semaphoresToWaitMerge.push_back(m_postProcessAA.getSemaphore());
	}
	else if(m_useReflections && m_reflections.isReady())
		semaphoresToWaitMerge.push_back(m_reflections.getSemaphore());
	else
		semaphoresToWaitMerge.push_back( m_toneMapping.getSemaphore());

	m_merge.submit(device, computeQueue, swapChainImageIndex, semaphoresToWaitMerge);
}

void SceneManager::resize(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue computeQueue, std::vector<Image*> swapChainImages)
{
	m_swapChainImages = swapChainImages;
	createSwapChainTextures(device, swapChainImages);
	
	m_camera.setAspect(swapChainImages[0]->getExtent().width / static_cast<float>(swapChainImages[0]->getExtent().height));

	/* -- HUD -- */
	createHUD(device, physicalDevice, graphicsQueue, nullptr, swapChainImages, true);

	/* -- GBuffer -- */
	if (!m_usePostProcessAA)
		m_gbuffer.resize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), swapChainImages[0]->getExtent());
	else 
		m_gbuffer.useDepthAsStorage(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_swapChainImages[0]->getExtent(), true);

	recoverGBufferImages(device, physicalDevice, graphicsQueue, nullptr);

	/* -- Skybox -- */
	m_skybox.resize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), graphicsQueue, m_swapChainImages[0]->getExtent());

	/* -- Shadows -- */
	m_shadows.resize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), graphicsQueue, &m_model, m_swapChainImages[0]->getExtent(),
		glm::vec4(1.5f, -5.0f, -1.0f, 1.0f), m_camera.getNear(), m_camera.getFar());

	/* -- AO -- */
	m_ambientOcclusion.recreate(device, physicalDevice, m_computeCommandPool.getCommandPool(), computeQueue, m_descriptorPool.getDescriptorPool(),
		m_camera.getProjection(), &m_gbufferTextures[0], &m_gbufferTextures[2]);
	
	/* -- PBR -- */
	createPBRComputePass(device, physicalDevice, computeQueue, true);

	/* -- Bloom -- */
	if (m_useBloom)
	{
		m_bloom.cleanup(device, m_computeCommandPool.getCommandPool());
		m_bloom.initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), computeQueue,
			m_useBloom ? m_bloom.getOutputTexture() : m_pbrCompute.getTextureOutput());
	}

	/* -- Tone Mapping -- */
	m_toneMapping.recreate(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), computeQueue, m_pbrCompute.getTextureOutput());

	/* -- Reflections -- */
	if(m_useReflections)
		m_reflections.recreate(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), computeQueue, m_toneMapping.getOutputTexture(),
			&m_gbufferTextures[0], &m_gbufferTextures[2], m_camera.getProjection(), static_cast<VkSampleCountFlagBits>(m_uboParamsData.sampleCount));

	/* -- MSAA -- */
	if (m_usePostProcessAA)
	{
		m_preDepthPass.cleanup(device, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool());

		glm::mat4 mvp = m_camera.getProjection() * m_camera.getViewMatrix() * m_model.getTransformation();
		m_preDepthPass.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), m_swapChainImages[0]->getExtent(),
			m_postProcessAASampleCount, &m_model, mvp, true);
		m_depthPassTexture.createFromImage(device, m_preDepthPass.getImage());

		createPostProcessAAComputePass(device, physicalDevice, computeQueue, true);
	}

	std::vector<Texture*> swapChainTextures = getSwapChainTextures();
	m_merge.recreate(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(),
		m_usePostProcessAA ? m_postProcessAA.getOutputTexture() : m_useReflections ? m_reflections.getTextureOutput() : m_toneMapping.getOutputTexture(), &m_HUDTextures[0],
		swapChainTextures, m_transitSwapchainToLayoutGeneral, m_transitSwapchainToLayoutPresent);
}

void SceneManager::cleanup(VkDevice device)
{
	m_model.cleanup(device);
	m_gbuffer.cleanup(device, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool());
	m_shadows.cleanup(device, m_graphicsCommandPool.getCommandPool());
	m_pbrCompute.cleanup(device, m_computeCommandPool.getCommandPool());
	m_ambientOcclusion.cleanup(device, m_computeCommandPool.getCommandPool());
	m_toneMapping.cleanup(device, m_computeCommandPool.getCommandPool());
	m_merge.cleanup(device, m_computeCommandPool.getCommandPool());
	m_skybox.cleanup(device, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool());
	m_graphicsCommandPool.cleanup(device);
	m_descriptorPool.cleanup(device);
}

VkSemaphore SceneManager::getLastRenderFinishedSemaphore()
{
	return m_merge.getSemaphore()->getSemaphore();

	/*if (!m_usePostProcessAA || !m_postProcessAACreated)
		return m_pbrCompute.getSemaphore()->getSemaphore();
	
	return m_postProcessAA.getSemaphore();*/
}

void SceneManager::createResources(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex)
{
	// Model
	m_model.loadFromFile(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, graphicsQueueMutex,
		"Models/sponza/sponza.obj", "Models/sponza");
}

void SceneManager::createUBOs(VkDevice device, VkPhysicalDevice physicalDevice)
{
	m_uboParamsData.drawShadows = 0;
}

void SceneManager::createSwapChainTextures(VkDevice device, const std::vector<Image*>& swapChainImages)
{
	m_swapchainTextures.resize(swapChainImages.size());
	for (size_t i(0); i < swapChainImages.size(); ++i)
	{
		m_swapchainTextures[i].createFromImage(device, swapChainImages[i]);
		m_swapchainTextures[i].createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);
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
}

void SceneManager::createPasses(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue,
                                std::mutex* graphicsQueueMutex, VkQueue computeQueue, const std::vector<Image*>& swapChainImages)
{
	/* -- HUD -- */
	createHUD(device, physicalDevice, graphicsQueue, graphicsQueueMutex, swapChainImages, false);

	/* -- Pre Depth Pass -- */
	glm::mat4 mvp = m_camera.getProjection() * m_camera.getViewMatrix() * m_model.getTransformation();
	if (m_usePostProcessAA)
	{
		m_preDepthPass.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), swapChainImages[0]->getExtent(), VK_SAMPLE_COUNT_4_BIT,
			&m_model, mvp, true);
		m_depthPassTexture.createFromImage(device, m_preDepthPass.getImage());
	}

	/* -- GBuffer -- */
	m_gbuffer.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), swapChainImages[0]->getExtent(), &m_model, m_camera.getViewMatrix(),
		m_camera.getProjection(), false);
	recoverGBufferImages(device, physicalDevice, graphicsQueue, graphicsQueueMutex);

	/* -- Skybox -- */
	graphicsQueueMutex->lock();
	m_skybox.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), graphicsQueue, swapChainImages[0]->getExtent());
	graphicsQueueMutex->unlock();

	/* -- Shadows -- */
	graphicsQueueMutex->lock();
	m_shadows.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, swapChainImages[0]->getExtent());
	graphicsQueueMutex->unlock();

	/* -- Ambient Occlusion -- */
	graphicsQueueMutex->lock();
	m_ambientOcclusion.initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), computeQueue, swapChainImages[0]->getExtent());
	graphicsQueueMutex->unlock();

	/* -- Compute Pass -- */
	graphicsQueueMutex->lock();
	createPBRComputePass(device, physicalDevice, computeQueue, false);
	graphicsQueueMutex->unlock();

	/* -- Tone Mapping */
	graphicsQueueMutex->lock();
	m_toneMapping.initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), computeQueue, m_pbrCompute.getTextureOutput());
	graphicsQueueMutex->unlock();

	/* -- Post process AA -- */
	if(m_usePostProcessAA)
		createPostProcessAAComputePass(device, physicalDevice, computeQueue, false);

	/* -- Merge Image + HUD -- */
	std::vector<Texture*> swapChainTextures = getSwapChainTextures();
	m_merge.initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), m_toneMapping.getOutputTexture(), &m_HUDTextures[0],
		swapChainTextures, m_transitSwapchainToLayoutGeneral, m_transitSwapchainToLayoutPresent);
}

void SceneManager::createHUD(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue,
                             std::mutex* graphicsQueueMutex, const std::vector<Image*>& swapChainImages, bool recreate)
{
	if(!recreate)
	{
		graphicsQueueMutex->lock();
		m_HUD.initialize(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), graphicsQueue, swapChainImages[0]->getExtent(),
			changeOptionCallback, this, m_hardwareCapabilities);
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

void SceneManager::recoverGBufferImages(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue,
	std::mutex* graphicsQueueMutex)
{
	// Get images from gbuffer
	std::vector<Image*> gbufferImages = m_gbuffer.getImages();
	m_gbufferTextures.resize(gbufferImages.size() - 1);
	for (size_t i(0); i < gbufferImages.size(); ++i)
	{
		if(graphicsQueueMutex)
			graphicsQueueMutex->lock();
		gbufferImages[i]->setImageLayout(device, m_graphicsCommandPool.getCommandPool(), graphicsQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		if(graphicsQueueMutex)
			graphicsQueueMutex->unlock();

		if(i == 0)
		{
			m_gbufferDepthTexture.createFromImage(device, gbufferImages[i]);
			m_gbufferDepthTexture.createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);
		}
		else
		{
			m_gbufferTextures[i - static_cast<size_t>(1)].createFromImage(device, gbufferImages[i]);
			m_gbufferTextures[i - static_cast<size_t>(1)].createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);
		}
	}
}

void SceneManager::createPBRComputePass(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue computeQueue,
                                        bool recreate)
{
	std::vector<Texture*> gBufferTextures(m_gbufferTextures.size());
	for (int i(0); i < m_gbufferTextures.size(); ++i)
		gBufferTextures[i] = &m_gbufferTextures[i];

	if (!recreate)
		m_pbrCompute.initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), gBufferTextures, m_shadows.getTexture(),
			m_ambientOcclusion.getTextureOutput(), m_skybox.getOutputTexture(), m_swapChainImages[0]->getExtent(), m_uboParamsData, computeQueue);
	else
	{
		m_pbrCompute.recreate(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), gBufferTextures, m_shadows.getTexture(),
			m_ambientOcclusion.getTextureOutput(), m_skybox.getOutputTexture(), m_swapChainImages[0]->getExtent(), computeQueue);
	}
}

void SceneManager::createPostProcessAAComputePass(VkDevice device, VkPhysicalDevice physicalDevice,
	VkQueue computeQueue, bool recreate)
{
	if (m_postProcessAACreated)
		m_postProcessAA.cleanup(device, m_computeCommandPool.getCommandPool());

	m_postProcessAA.initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), computeQueue, 
		m_useReflections ? m_reflections.getTextureOutput() : m_toneMapping.getOutputTexture(), 
		&m_gbufferDepthTexture, &m_depthPassTexture);

	m_postProcessAACreated = true;
}

std::vector<Texture*> SceneManager::getSwapChainTextures()
{
	std::vector<Texture*> swapChainTextures(m_swapChainImages.size());
	for (size_t i(0); i < m_swapChainImages.size(); ++i)
	{
		swapChainTextures[i] = &m_swapchainTextures[i];
	}

	return swapChainTextures;
}
