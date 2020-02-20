#pragma once

#include "ComputePass.h"

// Parameters
struct ParamsUBO
{
	glm::uint drawShadows;
	glm::uint sampleCount = 1;
	glm::uint useAO = 0;
};

class PBRCompute
{
public:
	PBRCompute() = default;
	~PBRCompute() = default;

	/*void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures,
		Texture* shadowMask, Texture* hudTexture, Texture* aoTexture, std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral,
		std::vector<Operation> transitSwapChainToLayoutPresent, ParamsUBO params);*/
	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures,
		Texture* shadowMask, Texture* aoTexture, Texture* skyboxTexture, VkExtent2D extentOutput, ParamsUBO params, VkQueue computeQueue);
	
	void submit(VkDevice device, VkQueue computeQueue, unsigned int swapChainImageIndex, std::vector<Semaphore*> semaphoresToWait, glm::vec3 lightDirection);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait, glm::vec3 cameraPosition);
	/*
	void recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures,
		Texture* shadowMask, Texture* hudTexture, Texture* aoTexture, std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral,
		std::vector<Operation> transitSwapChainToLayoutPresent);*/
	void recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures,
		Texture* shadowMask, Texture* aoTexture, Texture* skyboxTexture, VkExtent2D extent, VkQueue computeQueue);

	void cleanup(VkDevice device, VkCommandPool commandPool);

	void updateParameters(VkDevice device, ParamsUBO parameters);

	// Getters
public:
	Semaphore* getSemaphore() { return &m_computePassFinishedSemaphore; }
	Texture* getTextureOutput() { return &m_outputTexture; }

private:
	void createUBOs(VkDevice device, VkPhysicalDevice physicalDevice, ParamsUBO params);
	//void createPasses(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures,
	//	Texture* shadowMask, Texture* hudTexture, Texture* aoTexture, std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral,
	//	std::vector<Operation> transitSwapChainToLayoutPresent);
	void createPasses(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, std::vector<Texture*> gBufferTextures,
		Texture* shadowMask, Texture* aoTexture, Texture* skyboxTexture, VkQueue graphicsQueue, VkExtent2D extentOutput);
	
private:
	std::vector<ComputePass> m_computePasses;
	
	Texture m_outputTexture;
	bool m_useSwapChain = false;

	struct LightingUBO
	{
		glm::vec4 cameraPosition;

		glm::vec4 directionDirectionalLight;
		glm::vec4 colorDirectionalLight;
	};
	UniformBufferObject m_uboLighting;
	LightingUBO m_uboLightingData;

	UniformBufferObject m_uboParams;
	unsigned int m_sampleCount = 1;

	Semaphore m_computePassFinishedSemaphore;
};