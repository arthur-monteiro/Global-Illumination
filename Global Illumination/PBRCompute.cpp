#include "PBRCompute.h"

//void PBRCompute::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures,
//	Texture* shadowMask, Texture* hudTexture, Texture* aoTexture, std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral,
//	std::vector<Operation> transitSwapChainToLayoutPresent, ParamsUBO m_bokehPointGenerationParams)
//{
//	createUBOs(device, physicalDevice, m_bokehPointGenerationParams);
//	
//	createPasses(device, physicalDevice, commandPool, descriptorPool, std::move(gBufferTextures), shadowMask, hudTexture, aoTexture, std::move(swapChainTextures), std::move(transitSwapChainToLayoutGeneral),
//		std::move(transitSwapChainToLayoutPresent));
//
//	m_computePassFinishedSemaphore.initialize(device);
//	m_computePassFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
//
//	m_useSwapChain = true;
//}

void PBRCompute::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures, Texture* shadowMask, Texture* aoTexture, Texture* skyboxTexture,
	VkExtent2D extentOutput, ParamsUBO params, VkQueue computeQueue)
{
	createUBOs(device, physicalDevice, params);

	createPasses(device, physicalDevice, commandPool, descriptorPool, std::move(gBufferTextures), shadowMask, aoTexture, skyboxTexture, computeQueue, extentOutput);
	
	m_computePassFinishedSemaphore.initialize(device);
	m_computePassFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	m_useSwapChain = false;
}

void PBRCompute::submit(VkDevice device, VkQueue computeQueue, unsigned int swapChainImageIndex, std::vector<Semaphore*> semaphoresToWait, glm::vec3 lightDirection)
{
	m_uboLightingData.directionDirectionalLight = glm::vec4(lightDirection, 1.0f);
	m_uboLighting.updateData(device, &m_uboLightingData);

	if (swapChainImageIndex >= m_computePasses.size())
		swapChainImageIndex = 0;
	m_computePasses[swapChainImageIndex].submit(device, computeQueue, std::move(semaphoresToWait), m_computePassFinishedSemaphore.getSemaphore());
}

void PBRCompute::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait,
	glm::vec3 cameraPosition)
{
	submit(device, computeQueue, 0, std::move(semaphoresToWait), cameraPosition);
}


void PBRCompute::updateParameters(VkDevice device, ParamsUBO parameters)
{
	m_sampleCount = parameters.sampleCount;
	m_uboParams.updateData(device, &parameters);
}

void PBRCompute::createUBOs(VkDevice device, VkPhysicalDevice physicalDevice, ParamsUBO params)
{
	//m_uboLightingData.cameraPosition = glm::vec4(m_camera.getPosition(), 1.0f);
	m_uboLightingData.colorDirectionalLight = glm::vec4(10.0f);
	m_uboLightingData.directionDirectionalLight = glm::vec4(1.5f, -5.0f, -1.0f, 1.0f);
	m_uboLighting.initialize(device, physicalDevice, &m_uboLightingData, sizeof(m_uboLightingData));

	m_uboParams.initialize(device, physicalDevice, &params, sizeof(params));
}

//void PBRCompute::createPasses(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
//	VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures, Texture* shadowMask, Texture* hudTexture, Texture* aoTexture,
//	std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral,
//	std::vector<Operation> transitSwapChainToLayoutPresent)
//{
//	// Setting textures for compute pass
//	std::vector<std::pair<Texture*, TextureLayout>> texturesForComputePass;
//
//	// GBuffer
//	{
//		for (size_t i(0); i < gBufferTextures.size(); ++i)
//		{
//			TextureLayout textureLayout{};
//			textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
//			textureLayout.binding = i;
//
//			texturesForComputePass.emplace_back(gBufferTextures[i], textureLayout);
//		}
//	}
//
//	// RT Shadow
//	{
//		TextureLayout textureLayout{};
//		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
//		textureLayout.binding = gBufferTextures.size();
//
//		texturesForComputePass.emplace_back(shadowMask, textureLayout);
//	}
//
//	// HUD
//	{
//		TextureLayout textureLayout{};
//		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
//		textureLayout.binding = gBufferTextures.size() + 1; // GBuffer + RT Shadow
//
//		texturesForComputePass.emplace_back(hudTexture, textureLayout);
//	}
//
//	// HUD
//	{
//		TextureLayout textureLayout{};
//		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
//		textureLayout.binding = gBufferTextures.size() + 2; // GBuffer + RT Shadow + HUD
//
//		texturesForComputePass.emplace_back(aoTexture, textureLayout);
//	}
//
//	// UBOs
//	UniformBufferObjectLayout uboLightingLayout{};
//	uboLightingLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
//	uboLightingLayout.binding = 8;
//
//	UniformBufferObjectLayout uboParamsLayout{};
//	uboParamsLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
//	uboParamsLayout.binding = 9;
//
//	// Create compute passes
//	m_computePasses.resize(swapChainTextures.size());
//	for (size_t i(0); i < m_computePasses.size(); ++i)
//	{
//		std::vector<std::pair<Texture*, TextureLayout>> textures = texturesForComputePass;
//
//		TextureLayout textureLayout{};
//		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
//		textureLayout.binding = textures.size();
//
//		textures.emplace_back(swapChainTextures[i], textureLayout);
//
//		std::string shaderPath = "Shaders/compute/comp.spv";
//		if (m_sampleCount > 1)
//			shaderPath = "Shaders/compute/compMS.spv";
//
//		m_computePasses[i].initialize(device, physicalDevice, commandPool, descriptorPool, swapChainTextures[0]->getImage()->getExtent(),
//			{ 16, 16, 1 }, shaderPath, { { &m_uboLighting, uboLightingLayout}, { &m_uboParams, uboParamsLayout} },
//			textures, { transitSwapChainToLayoutGeneral[i] }, { transitSwapChainToLayoutPresent[i] });
//	}
//}

void PBRCompute::createPasses(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures, Texture* shadowMask, Texture* aoTexture, Texture* skyboxTexture, VkQueue computeQueue,
	VkExtent2D extentOutput)
{
	m_outputTexture.create(device, physicalDevice, extentOutput, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_outputTexture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	m_outputTexture.createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);

	// Setting textures for compute pass
	std::vector<std::pair<Texture*, TextureLayout>> texturesForComputePass;

	// GBuffer
	{
		for (size_t i(0); i < gBufferTextures.size(); ++i)
		{
			TextureLayout textureLayout{};
			textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
			textureLayout.binding = i;

			texturesForComputePass.emplace_back(gBufferTextures[i], textureLayout);
		}
	}

	// RT Shadow
	{
		TextureLayout textureLayout{};
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = gBufferTextures.size();

		texturesForComputePass.emplace_back(shadowMask, textureLayout);
	}

	// HUD
	//{
	//	TextureLayout textureLayout{};
	//	textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	//	textureLayout.binding = gBufferTextures.size() + 1; // GBuffer + RT Shadow

	//	texturesForComputePass.emplace_back(hudTexture, textureLayout);
	//}

	// AO
	{
		TextureLayout textureLayout{};
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = gBufferTextures.size() + 1; // GBuffer + RT Shadow

		texturesForComputePass.emplace_back(aoTexture, textureLayout);
	}

	// Skybox
	{
		TextureLayout textureLayout{};
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = gBufferTextures.size() + 2; // GBuffer + RT Shadow + AO

		texturesForComputePass.emplace_back(skyboxTexture, textureLayout);
	}

	// UBOs
	UniformBufferObjectLayout uboLightingLayout{};
	uboLightingLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboLightingLayout.binding = 8;

	UniformBufferObjectLayout uboParamsLayout{};
	uboParamsLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboParamsLayout.binding = 9;

	// Create compute passes
	m_computePasses.resize(1);
	std::vector<std::pair<Texture*, TextureLayout>> textures = texturesForComputePass;

	TextureLayout textureLayout{};
	textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureLayout.binding = textures.size();

	textures.emplace_back(&m_outputTexture, textureLayout);

	std::string shaderPath = "Shaders/compute/comp.spv";
	if (m_sampleCount > 1)
		shaderPath = "Shaders/compute/compMS.spv";

	m_computePasses[0].initialize(device, physicalDevice, commandPool, descriptorPool, extentOutput,
		{ 16, 16, 1 }, shaderPath, { { &m_uboLighting, uboLightingLayout}, { &m_uboParams, uboParamsLayout} },
		textures, { }, { });
}


void PBRCompute::cleanup(VkDevice device, VkCommandPool commandPool)
{
	for (size_t i(0); i < m_computePasses.size(); ++i)
	{
		m_computePasses[i].cleanup(device, commandPool);
	}
}

//void PBRCompute::recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
//	VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures, Texture* shadowMask, Texture* hudTexture, Texture* aoTexture,
//	std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral,
//	std::vector<Operation> transitSwapChainToLayoutPresent)
//{
//	cleanup(device, commandPool);
//
//	createPasses(device, physicalDevice, commandPool, descriptorPool, std::move(gBufferTextures), shadowMask, hudTexture, aoTexture, std::move(swapChainTextures),
//		std::move(transitSwapChainToLayoutGeneral), std::move(transitSwapChainToLayoutPresent));
//}

void PBRCompute::recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures, Texture* shadowMask, Texture* aoTexture, Texture* skyboxTexture,
	VkExtent2D extent, VkQueue computeQueue)
{
	cleanup(device, commandPool);

	createPasses(device, physicalDevice, commandPool, descriptorPool, std::move(gBufferTextures), shadowMask, aoTexture, skyboxTexture, computeQueue, extent);
}

