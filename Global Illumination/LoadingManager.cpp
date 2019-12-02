#include "LoadingManager.h"

LoadingManager::~LoadingManager()
= default;

bool LoadingManager::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, std::vector<Image*> swapChainImages)
{
	// Command Pool + Descriptor Pool
	m_uniqueCommandPool.initialize(device, physicalDevice, surface);
	m_uniqueDescriptorPool.initialize(device);

	// Textures
	m_fullScreenLoadingTexture.createFromFile(device, physicalDevice, m_uniqueCommandPool.getCommandPool(), graphicsQueue, "Textures/loading_screen.png");
	m_fullScreenLoadingTexture.createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);

	m_loadingLogoTexture.createFromFile(device, physicalDevice, m_uniqueCommandPool.getCommandPool(), graphicsQueue, "Textures/loading_logo.jpg");
	m_loadingLogoTexture.createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);

	// Quads
	m_fullScreenQuad.addMeshFromVertices(device, physicalDevice, m_uniqueCommandPool.getCommandPool(), graphicsQueue,
		{
			{ glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f) }, // bot left
			{ glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f) }, // top left
			{ glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 0.0f) }, // bot right
			{ glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f) } // top right
		},
		{
			0, 1, 2,
			1, 3, 2
		});

	m_loadingLogoQuad.addMeshFromVertices(device, physicalDevice, m_uniqueCommandPool.getCommandPool(), graphicsQueue,
		{
			{ glm::vec2(1.0f - m_logoWidth, 1.0f), glm::vec2(0.0f, 1.0f) }, // bot left
			{ glm::vec2(1.0f - m_logoWidth, 1.0f - m_logoWidth), glm::vec2(0.0f, 0.0f) }, // top left
			{ glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f) }, // bot right
			{ glm::vec2(1.0f, 1.0f - m_logoWidth), glm::vec2(1.0f, 0.0f) } // top right
		},
		{
			0, 1, 2,
			1, 3, 2
		});

	// Quad renderer
	TextureLayout textureLayout;
	textureLayout.binding = 0;
	textureLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;

	{
		m_fullScreenQuadRenderer.initialize(device, "Shaders/loading/fullScreenQuadVertex.spv", "Shaders/loading/fullScreenQuadFragment.spv",
			{ Vertex2DTextured::getBindingDescription(0) }, Vertex2DTextured::getAttributeDescriptions(0), { }, { textureLayout }, { true });
		std::vector<VertexBuffer> modelVertexBuffers = m_fullScreenQuad.getVertexBuffers();
		for (int i(0); i < modelVertexBuffers.size(); ++i)
			m_fullScreenQuadRenderer.addMesh(device, m_uniqueDescriptorPool.getDescriptorPool(), modelVertexBuffers[i],
				{ }, { { &m_fullScreenLoadingTexture, textureLayout } });
	}

	UniformBufferObjectLayout uboLayoutLogoOpacity;
	uboLayoutLogoOpacity.binding = 1;
	uboLayoutLogoOpacity.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;

	UniformBufferObjectLayout uboLayoutLogoOffset;
	uboLayoutLogoOffset.binding = 2;
	uboLayoutLogoOffset.accessibility = VK_SHADER_STAGE_VERTEX_BIT;

	m_uboLogoOpacity.initialize(device, physicalDevice, &m_logoOpacity, sizeof(float));

	float screenAspect = swapChainImages[0]->getExtent().width / (float)swapChainImages[0]->getExtent().height;
	float offset = m_logoWidth * (screenAspect - 1.0f);
	m_uboLogoOffset.initialize(device, physicalDevice, &offset, sizeof(float));

	{
		m_loadingLogoQuadRenderer.initialize(device, "Shaders/loading/logoQuadVertex.spv", "Shaders/loading/logoQuadFragment.spv",
			{ Vertex2DTextured::getBindingDescription(0) }, Vertex2DTextured::getAttributeDescriptions(0), { uboLayoutLogoOpacity, uboLayoutLogoOffset }, { textureLayout }, { true });
		std::vector<VertexBuffer> modelVertexBuffers = m_loadingLogoQuad.getVertexBuffers();
		for (int i(0); i < modelVertexBuffers.size(); ++i)
			m_loadingLogoQuadRenderer.addMesh(device, m_uniqueDescriptorPool.getDescriptorPool(), modelVertexBuffers[i],
				{ { &m_uboLogoOpacity, uboLayoutLogoOpacity }, { &m_uboLogoOffset, uboLayoutLogoOffset } }, { { &m_loadingLogoTexture, textureLayout } });
	}

	/* Main Render Pass */
	// Attachments -> depth + color
    m_attachments.resize(2);
	m_attachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	m_attachments[1].initialize(swapChainImages[0]->getFormat(), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	// Init with swapchain images
	m_mainRenderPass.initialize(device, physicalDevice, surface, m_uniqueCommandPool.getCommandPool(), m_attachments, swapChainImages);

	// Fill command buffers
	m_clearValues.resize(2);
	m_clearValues[0] = { 1.0f };
	m_clearValues[1] = { 1.0f, 0.0f, 0.0f, 1.0f };
    for(int i(0); i < swapChainImages.size(); ++i)
		m_mainRenderPass.fillCommandBuffer(device, i, m_clearValues, { &m_fullScreenQuadRenderer, &m_loadingLogoQuadRenderer });

	return true;
}

bool LoadingManager::submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, Semaphore * imageAvailableSemaphore)
{
	long long millisecondOffset = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_timerStart).count();

	float logoOpacity = (glm::exp(glm::sin(millisecondOffset / (float)m_loadingLogoLoopMillisecond)) / 2.0f - 0.18f) / 1.18f;
	m_uboLogoOpacity.updateData(device, &logoOpacity);

    m_mainRenderPass.submit(device, graphicsQueue, swapChainImageIndex, { imageAvailableSemaphore });

    return true;
}

void LoadingManager::resize(VkDevice device, VkPhysicalDevice physicalDevice, std::vector<Image*> swapChainImages)
{
	float screenAspect = swapChainImages[0]->getExtent().width / (float)swapChainImages[0]->getExtent().height;
	float offset = m_logoWidth * (screenAspect - 1.0f);
	m_uboLogoOffset.updateData(device, &offset);

	m_mainRenderPass.resize(device, physicalDevice, m_uniqueCommandPool.getCommandPool(), m_attachments, swapChainImages);
	m_fullScreenQuadRenderer.destroyPipeline(device);
	m_loadingLogoQuadRenderer.destroyPipeline(device);
	for (int i(0); i < swapChainImages.size(); ++i)
		m_mainRenderPass.fillCommandBuffer(device, i, m_clearValues, { &m_fullScreenQuadRenderer, &m_loadingLogoQuadRenderer });
}

void LoadingManager::cleanup(VkDevice device)
{
	m_mainRenderPass.cleanup(device, m_uniqueCommandPool.getCommandPool());
	m_fullScreenQuadRenderer.cleanup(device, m_uniqueDescriptorPool.getDescriptorPool());
	m_fullScreenLoadingTexture.cleanup(device);
	m_uniqueCommandPool.cleanup(device);
	m_uniqueDescriptorPool.cleanup(device);
	m_fullScreenQuad.cleanup(device);
}

VkSemaphore LoadingManager::getLastRenderFinishedSemaphore()
{
    return m_mainRenderPass.getRenderCompleteSemaphore()->getSemaphore();
}
