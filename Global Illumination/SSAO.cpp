#include "SSAO.h"
#include <random>

void SSAO::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue,
                      VkDescriptorPool descriptorPool, glm::mat4 projection, Texture* viewPosTexture, Texture* normalTexture)
{
	const std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
	std::random_device rd;
	std::default_random_engine generator(rd());
	std::array<glm::vec4, 16> ssaoKernel;
	for (unsigned int i = 0; i < 16; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) // normal is (0, 0, 1)
		);
		sample = glm::normalize(sample); // fit point into hemisphere (r = 1)
		sample *= randomFloats(generator);
		float scale = static_cast<float>(i) / 64.0f;
		scale = glm::mix(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel[i] = glm::vec4(sample, 0.0f);
	}

	std::array<glm::vec4, 16> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise[i] = glm::vec4(noise, 0.0f);
	}

	m_uboData.samples = ssaoKernel;
	m_uboData.noise = ssaoNoise;
	m_uboData.projection = projection;
	m_ubo.initialize(device, physicalDevice, &m_uboData, sizeof(m_uboData));

	UniformBufferObjectLayout uboLayout{};
	uboLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboLayout.binding = 3;

	TextureLayout worldPosLayout{};
	worldPosLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	worldPosLayout.binding = 0;

	TextureLayout normalLayout{};
	normalLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	normalLayout.binding = 1;

	TextureLayout outputLayout{};
	outputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	outputLayout.binding = 2;

	m_outputTexture.create(device, physicalDevice, viewPosTexture->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_outputTexture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	std::string shaderPath;
	if (viewPosTexture->getImage()->getSampleCount() != VK_SAMPLE_COUNT_1_BIT)
		shaderPath = "Shaders/ambientOcclusion/compSSAOMS.spv";
	else
		shaderPath = "Shaders/ambientOcclusion/compSSAO.spv";

	m_computePass.initialize(device, physicalDevice, commandPool, descriptorPool, viewPosTexture->getImage()->getExtent(), { 16, 16, 1 }, 
		shaderPath, { { &m_ubo, uboLayout } }, {
			{ viewPosTexture, worldPosLayout },
			{ normalTexture, normalLayout },
			{ &m_outputTexture, outputLayout }
		}, {}, {});

	m_renderFinishedSemaphore.initialize(device);
	m_renderFinishedSemaphore.setPipelineStage(VK_SHADER_STAGE_COMPUTE_BIT);

	m_blur.initialize(device, physicalDevice, commandPool, descriptorPool, computeQueue, &m_outputTexture, 1);
}

void SSAO::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait)
{
	m_computePass.submit(device, computeQueue, std::move(semaphoresToWait), m_renderFinishedSemaphore.getSemaphore());
	m_blur.submit(device, computeQueue, { &m_renderFinishedSemaphore });
}

void SSAO::cleanup(VkDevice device, VkCommandPool commandPool)
{
	m_computePass.cleanup(device, commandPool);
	m_renderFinishedSemaphore.cleanup(device);
	m_blur.cleanup(device, commandPool);
	m_outputTexture.cleanup(device);
	m_ubo.cleanup(device);
}

void SSAO::changePower(VkDevice device, int power)
{
	m_uboData.power.x = static_cast<float>(power);
	m_ubo.updateData(device, &m_uboData);
}
