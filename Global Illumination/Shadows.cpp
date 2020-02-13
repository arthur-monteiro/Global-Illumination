#include "Shadows.h"

#include <utility>

void Shadows::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent)
{
	m_renderFinishedSemaphore.initialize(device);

	createDefaultTexture(device, physicalDevice, commandPool, computeQueue, extent);
}

bool Shadows::changeShadowType(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model, glm::mat4 mvp, VkExtent2D extentOutput,
	const std::wstring& newType)
{
	ShadowType requestedType{};
	if (newType == L"No" || newType == L"")
		requestedType = ShadowType::NO;
	else if (newType == L"NVidia Ray Tracing")
		requestedType = ShadowType::RTX;

	if (m_shadowType == requestedType)
		return false;

	if(newType == L"NVidia Ray Tracing")
	{
		m_shadowType = ShadowType::RTX;
		m_renderFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		m_rtShadows.initialize(device, physicalDevice, commandPool, graphicsQueue, model, mvp, extentOutput);
	}
	else
	{
		m_shadowType = ShadowType::NO;
		m_rtShadows.cleanup(device, commandPool);
	}

	return true;
}

void Shadows::submit(VkDevice device, VkQueue queue, std::vector<Semaphore*> waitSemaphores, glm::mat4 viewInverse, glm::mat4 projInverse)
{
	if(m_shadowType == ShadowType::RTX)
	{
		m_rtShadows.submit(device, queue, std::move(waitSemaphores), m_renderFinishedSemaphore.getSemaphore(), viewInverse, projInverse);
	}
}

void Shadows::resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,
	ModelPBR* model, VkExtent2D extentOutput)
{
	if(m_shadowType == ShadowType::RTX)
		m_rtShadows.resize(device, physicalDevice, commandPool, graphicsQueue, model, extentOutput);
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

	// Need to return a valid image
	return &m_defaultTexture;
}

void Shadows::createDefaultTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent)
{
	m_defaultTexture.create(device, physicalDevice, extent, VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_defaultTexture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
