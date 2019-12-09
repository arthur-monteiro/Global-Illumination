#pragma once

#include "Image.h"
#include "Semaphore.h"
#include "Command.h"
#include "Pipeline.h"
#include "Operation.h"

class ComputePass
{
public:
	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool,
		VkExtent2D extent, VkExtent3D dispatchGroups, std::string computeShader, 
		std::vector<std::pair<UniformBufferObject*, UniformBufferObjectLayout>> ubos, std::vector<std::pair<Texture*, TextureLayout>> textures,
		std::vector<Operation> operationsBefore, std::vector<Operation> operationsAfter);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> waitSemaphores, VkSemaphore signalSemaphore);
	void cleanup(VkDevice device);

private:
	Command m_command;
	Pipeline m_pipeline;

private:
	void fillCommandBufferWithOperation(VkCommandBuffer commandBuffer, std::vector<Operation> operations);
};