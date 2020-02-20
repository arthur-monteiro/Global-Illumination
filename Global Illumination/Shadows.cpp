#include "Shadows.h"

#include <utility>

void Shadows::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent)
{
	m_renderFinishedSemaphore.initialize(device);

	createDefaultTexture(device, physicalDevice, commandPool, computeQueue, extent);
}

bool Shadows::changeShadowType(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, 
	VkQueue graphicsQueue, ModelPBR* model, glm::mat4 mvp, glm::vec3 sunDir, float cameraNear, float cameraFar, VkExtent2D extentOutput,
	const std::wstring& newType)
{
	ShadowType requestedType{};
	if (newType == L"No" || newType == L"")
		requestedType = ShadowType::NO;
	else if (newType == L"NVidia Ray Tracing")
		requestedType = ShadowType::RTX;
	else if (newType == L"CSM")
		requestedType = ShadowType::CSM;

	if (m_shadowType == requestedType)
		return false;

	// Destroy
	/*if (m_shadowType == ShadowType::RTX)
		m_rtShadows.cleanup(device, commandPool);
	else */if (m_shadowType == ShadowType::CSM)
		m_csm.cleanup(device, commandPool, descriptorPool);

	// Create
	if(newType == L"NVidia Ray Tracing")
	{
		m_shadowType = ShadowType::RTX;
		m_renderFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		m_rtShadows.initialize(device, physicalDevice, commandPool, graphicsQueue, model, mvp, extentOutput);
	}
	else if (newType == L"CSM")
	{
		m_shadowType = ShadowType::CSM;
		m_csm.initialize(device, physicalDevice, commandPool, descriptorPool, graphicsQueue, extentOutput, model, sunDir, cameraNear, cameraFar);
	}
	else
	{
		m_shadowType = ShadowType::NO;
	}

	return true;
}

void Shadows::submit(VkDevice device, VkQueue graphicsQueue, std::vector<Semaphore*> waitSemaphores, glm::mat4 viewInverse, glm::mat4 projInverse, glm::mat4 view, glm::mat4 model, glm::mat4 projection,
	float cameraNear, float cameraFOV, glm::vec3 lightDir, glm::vec3 cameraPosition, glm::vec3 cameraOrientation)
{
	if(m_shadowType == ShadowType::RTX)
	{
		m_rtShadows.submit(device, graphicsQueue, std::move(waitSemaphores), m_renderFinishedSemaphore.getSemaphore(), viewInverse, projInverse);
	}
	else if (m_shadowType == ShadowType::CSM)
	{
		m_csm.submit(device, graphicsQueue, view, model, projection, cameraNear, cameraFOV, lightDir, cameraPosition, cameraOrientation);
	}
}

void Shadows::resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue,
	ModelPBR* model, VkExtent2D extentOutput, glm::vec3 lightDir, float cameraNear, float cameraFar)
{
	if(m_shadowType == ShadowType::RTX)
		m_rtShadows.resize(device, physicalDevice, commandPool, graphicsQueue, model, extentOutput);
	else if(m_shadowType == ShadowType::CSM)
	{
		m_csm.cleanup(device, commandPool, descriptorPool);
		m_csm.initialize(device, physicalDevice, commandPool, descriptorPool, graphicsQueue, extentOutput, model, lightDir, cameraNear, cameraFar);
	}
}

void Shadows::changeRTSampleCount(VkDevice device, unsigned int sampleCount)
{
	m_rtShadows.changeSampleCount(device, sampleCount);
}

void Shadows::cleanup(VkDevice device, VkCommandPool commandPool)
{
	if (m_shadowType == ShadowType::RTX)
		m_rtShadows.cleanup(device, commandPool);
	else
		m_defaultTexture.cleanup(device);
}

Texture* Shadows::getTexture()
{
	if (m_shadowType == ShadowType::RTX)
	{
		return m_rtShadows.getTexture();
	}
	else if (m_shadowType == ShadowType::CSM)
	{
		return m_csm.getOutputTexture();
	}

	// Need to return a valid image
	return &m_defaultTexture;
}

Semaphore* Shadows::getRenderCompleteSemaphore()
{
	if (m_shadowType == ShadowType::RTX)
	{
		return &m_renderFinishedSemaphore;
	}
	else if (m_shadowType == ShadowType::CSM)
	{
		return m_csm.getSemaphore();
	}
}

void Shadows::createDefaultTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent)
{
	m_defaultTexture.create(device, physicalDevice, extent, VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_defaultTexture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
