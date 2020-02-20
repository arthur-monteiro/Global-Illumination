#include "Blur.h"

void Blur::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* inputTexture, int blurAmount)
{
	m_texture.create(device, physicalDevice, inputTexture->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, inputTexture->getImage()->getFormat(), VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT);
	m_texture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	m_computePasses.resize(2 * blurAmount);

	TextureLayout inputLayout{};
	inputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	inputLayout.binding = 0;

	TextureLayout resultLayout{};
	resultLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	resultLayout.binding = 1;

	for (int i(0); i < m_computePasses.size(); ++i)
	{
		std::string shaderPath;
		if (i % 2 == 0)
			shaderPath = "Shaders/blur/vertical.spv";
		else
			shaderPath = "Shaders/blur/horizontal.spv";

		std::vector<std::pair<Texture*, TextureLayout>> textures;
		if (i % 2 == 0)
			textures = { { inputTexture, inputLayout}, { &m_texture, resultLayout} };
		else
			textures = { { &m_texture, inputLayout}, { inputTexture, resultLayout} };

		m_computePasses[i].first.initialize(device, physicalDevice, commandPool, descriptorPool, inputTexture->getImage()->getExtent(), { 16, 16, 1 }, shaderPath, {},
			textures, {}, {});
		m_computePasses[i].second.initialize(device);
		m_computePasses[i].second.setPipelineStage(VK_SHADER_STAGE_COMPUTE_BIT);
	}
}

void Blur::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait)
{
	m_computePasses[0].first.submit(device, computeQueue, std::move(semaphoresToWait), m_computePasses[0].second.getSemaphore());
	for (int i(1); i < m_computePasses.size(); ++i)
	{
		m_computePasses[i].first.submit(device, computeQueue, { &m_computePasses[i - 1].second }, m_computePasses[i].second.getSemaphore());
	}
}

void Blur::cleanup(VkDevice device, VkCommandPool commandPool)
{
	for (int i(0); i < m_computePasses.size(); ++i)
	{
		m_computePasses[i].first.cleanup(device, commandPool);
		m_computePasses[i].second.cleanup(device);
	}
}
