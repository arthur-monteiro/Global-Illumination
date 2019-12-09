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

	// Model
    m_model.loadFromFile(device, physicalDevice, m_graphicsCommandPool.getCommandPool(), graphicsQueue, graphicsQueueMutex,
            "Models/sponza/sponza.obj", "Models/sponza");

	// GBuffer
	glm::mat4 mvp = m_camera.getProjection() * m_camera.getViewMatrix() * m_model.getTransformation();
    m_gbuffer.initialize(device, physicalDevice, surface, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), swapChainImages[0]->getExtent(), &m_model, mvp);

	// Get images from gbuffer
	std::vector<Image*> gbufferImages = m_gbuffer.getImages();
	m_gbufferTextures.resize(gbufferImages.size() - 1);
	for (int i(1); i < gbufferImages.size(); ++i)
	{
		graphicsQueueMutex->lock();
		gbufferImages[i]->setImageLayout(device, m_graphicsCommandPool.getCommandPool(), graphicsQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		graphicsQueueMutex->unlock();

		m_gbufferTextures[i - 1].createFromImage(device, gbufferImages[i]);
		m_gbufferTextures[i - 1].createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);
	}

	std::vector<std::pair<Texture*, TextureLayout>> texturesForComputePass;
	for (int i(1); i < gbufferImages.size(); ++i)
	{
		TextureLayout textureLayout;
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = i - 1;

		texturesForComputePass.push_back({ &m_gbufferTextures[i - 1], textureLayout});
	}

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
	uboLightingLayout.binding = 5;

	m_uboLightingData.cameraPosition = glm::vec4(m_camera.getPosition(), 1.0f);
	m_uboLightingData.colorDirectionalLight = glm::vec4(10.0f);
	m_uboLightingData.directionDirectionalLight = glm::vec4(1.5f, -5.0f, -1.0f, 1.0f);
	m_uboLighting.initialize(device, physicalDevice, &m_uboLightingData, sizeof(m_uboLightingData));

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

		textures.push_back({ &m_swapchainTextures[i], textureLayout });

		m_computePasses[i].initialize(device, physicalDevice, m_computeCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool(), swapChainImages[0]->getExtent(),
			{ 16, 16, 1 }, "Shaders/compute/comp.spv", { { &m_uboLighting, uboLightingLayout} }, textures, { m_transitSwapchainToLayoutGeneral[i] }, { m_transitSwapchainToLayoutPresent[i] });
	}

	m_computePassFinishedSemaphore.initialize(device);
	m_computePassFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    m_loadingState = 1.0f;
}

void SceneManager::submit(VkDevice device, GLFWwindow* window, VkQueue graphicsQueue, VkQueue computeQueue, uint32_t swapChainImageIndex, Semaphore * imageAvailableSemaphore)
{
	m_camera.update(window);
    glm::mat4 mvp = m_camera.getProjection() * m_camera.getViewMatrix() * m_model.getTransformation();
	m_gbuffer.submit(device, graphicsQueue, mvp, m_model.getTransformation());

	m_uboLightingData.cameraPosition = glm::vec4(m_camera.getPosition(), 1.0f);
	m_uboLighting.updateData(device, &m_uboLightingData);
	m_computePasses[swapChainImageIndex].submit(device, computeQueue, { m_gbuffer.getRenderCompleteSemaphore(), imageAvailableSemaphore }, m_computePassFinishedSemaphore.getSemaphore());
}

void SceneManager::cleanup(VkDevice device)
{
	m_model.cleanup(device);
	m_gbuffer.cleanup(device, m_graphicsCommandPool.getCommandPool(), m_descriptorPool.getDescriptorPool());
	m_graphicsCommandPool.cleanup(device);
	m_descriptorPool.cleanup(device);
}

VkSemaphore SceneManager::getLastRenderFinishedSemaphore()
{
	return m_computePassFinishedSemaphore.getSemaphore();
}
