#pragma once

#include "Image.h"
#include "Semaphore.h"
#include "Command.h"
#include "Pipeline.h"

class ComputePass
{
public:
	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool,
		VkExtent2D extent, VkExtent3D dispatchGroups, std::string computeShader, 
		std::vector<std::pair<UniformBufferObject*, UniformBufferObjectLayout>> ubos, std::vector<std::pair<Texture*, TextureLayout>> textures);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> waitSemaphores);
	void cleanup(VkDevice device);

	Semaphore getRenderFinishedSemaphore() { return m_renderCompleteSemaphore; }
private:
	Command m_command;
	Pipeline m_pipeline;
	Semaphore m_renderCompleteSemaphore;
};