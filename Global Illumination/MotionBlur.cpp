#include "MotionBlur.h"

void MotionBlur::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* viewPos, Texture* color)
{
	m_outputTexture.create(device, physicalDevice, viewPos->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_outputTexture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	m_velocityTexture.create(device, physicalDevice, viewPos->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R32G32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_velocityTexture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	m_ubo.initialize(device, physicalDevice, &m_uboData, sizeof(UBOMotionBlur));
	
	TextureLayout textureViewPosInputLayout{};
	textureViewPosInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureViewPosInputLayout.binding = 0;

	TextureLayout textureColorInputLayout{};
	textureColorInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureColorInputLayout.binding = 1;

	TextureLayout textureVelocityLayout{};
	textureVelocityLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureVelocityLayout.binding = 2;

	TextureLayout textureOutputLayout{};
	textureOutputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureOutputLayout.binding = 3;

	UniformBufferObjectLayout uboLayout;
	uboLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboLayout.binding = 4;

	std::string shaderPath = "Shaders/postProcess/motionBlur.spv";

	m_computePass.initialize(device, physicalDevice, commandPool, descriptorPool, viewPos->getImage()->getExtent(), { 16, 16, 1 }, shaderPath, 
		{ { &m_ubo, uboLayout } },
		{ { viewPos, textureViewPosInputLayout}, { color, textureColorInputLayout}, { &m_velocityTexture, textureVelocityLayout }, { &m_outputTexture, textureOutputLayout} },
		{}, {});

	m_semaphore.initialize(device);
	m_semaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void MotionBlur::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait, glm::mat4 invView, glm::mat4 previousView, glm::mat4 projection)
{	
	m_uboData.invView = invView;
	m_uboData.previousView = previousView;
	m_uboData.projection = projection;
	m_uboData.timeBetweenFrames = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_previousTimePoint).count();
	m_ubo.updateData(device, &m_uboData);
	
	m_computePass.submit(device, computeQueue, std::move(semaphoresToWait), m_semaphore.getSemaphore());

	m_previousTimePoint = std::chrono::steady_clock::now();
}

void MotionBlur::cleanup(VkDevice device, VkCommandPool commandPool)
{
	m_semaphore.cleanup(device);
	m_computePass.cleanup(device, commandPool);
	m_outputTexture.cleanup(device);
}
