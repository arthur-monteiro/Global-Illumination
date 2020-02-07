#include "Shadows.h"

#include <utility>

void Shadows::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                         VkQueue graphicsQueue, ModelPBR* model, glm::mat4 mvp, VkExtent2D extentOutput)
{
	m_renderFinishedSemaphore.initialize(device);

	m_rtShadows.initialize(device, physicalDevice, commandPool, graphicsQueue, model, mvp, extentOutput);
}

void Shadows::changeShadowType(const std::wstring& newType)
{
	if(newType == L"NVidia Ray Tracing")
	{
		m_shadowType = ShadowType::RTX;
		m_renderFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	}
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
	m_rtShadows.resize(device, physicalDevice, commandPool, graphicsQueue, model, extentOutput);
}

void Shadows::changeRTSampleCount(VkDevice device, unsigned sampleCount)
{
	m_rtShadows.changeSampleCount(device, sampleCount);
}

void Shadows::cleanup(VkDevice device, VkCommandPool commandPool)
{
	m_rtShadows.cleanup(device, commandPool);
}

Image* Shadows::getImage()
{
	if (m_shadowType == ShadowType::RTX)
	{
		return m_rtShadows.getImage();
	}

	// Need to return a valid image
	return m_rtShadows.getImage();
}
