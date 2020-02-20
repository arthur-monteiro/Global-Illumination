#include "SSR.h"

void SSR::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* inputShaded, Texture* inputViewPos,
	Texture* inputNormal, glm::mat4 projection)
{
	m_outputTexture.create(device, physicalDevice, inputShaded->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_outputTexture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	m_ubo.initialize(device, physicalDevice, &projection, sizeof(glm::mat4));

	TextureLayout textureInputShadedLayout{};
	textureInputShadedLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureInputShadedLayout.binding = 0;

	TextureLayout textureInputViewPosLayout{};
	textureInputViewPosLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureInputViewPosLayout.binding = 1;

	TextureLayout textureInputNormalLayout{};
	textureInputNormalLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureInputNormalLayout.binding = 2;

	TextureLayout textureOutputLayout{};
	textureOutputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureOutputLayout.binding = 3;

	UniformBufferObjectLayout uboLayout{};
	uboLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboLayout.binding = 4;

	std::string shaderPath = "Shaders/reflections/ssr.spv";

	m_pass.initialize(device, physicalDevice, commandPool, descriptorPool, inputShaded->getImage()->getExtent(),
		{ 16, 16, 1 }, shaderPath, { { &m_ubo, uboLayout} },{
			{ inputShaded, textureInputShadedLayout},
			{ inputViewPos, textureInputViewPosLayout},
			{ inputNormal, textureInputNormalLayout},
			{ &m_outputTexture, textureOutputLayout}
		}, {}, {});

	m_semaphore.initialize(device);
	m_semaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void SSR::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait)
{
	m_pass.submit(device, computeQueue, std::move(semaphoresToWait), m_semaphore.getSemaphore());
}

void SSR::cleanup(VkDevice device, VkCommandPool commandPool)
{
	m_pass.cleanup(device, commandPool);
	m_outputTexture.cleanup(device);
	m_semaphore.cleanup(device);
}
