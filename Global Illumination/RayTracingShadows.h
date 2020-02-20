#pragma once

#include "VulkanHelper.h"
#include "ModelPBR.h"
#include "AccelerationStructure.h"
#include "vulkannv/nv_helpers_vk/DescriptorSetGenerator.h"
#include "UniformBufferObject.h"
#include "RayTracingPipeline.h"
#include "vulkannv/nv_helpers_vk/ShaderBindingTableGenerator.h"
#include "Command.h"

class RayTracingShadows
{
public:
	RayTracingShadows() = default;
	~RayTracingShadows();

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model, glm::mat4 mvp, VkExtent2D extentOutput);
	void submit(VkDevice device, VkQueue queue, std::vector<Semaphore*> waitSemaphores, VkSemaphore signalSemaphore, glm::mat4 viewInverse, glm::mat4 projInverse);

	void resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model, VkExtent2D extentOutput);
	void changeSampleCount(VkDevice device, unsigned int sampleCount);

	void cleanup(VkDevice device, VkCommandPool commandPool);

	Texture* getTexture() { return &m_textureTarget; }

private:
	void fillCommandBuffer(VkExtent2D extent);
	
	void createRaytracingDescriptorSet(VkDevice device, std::vector<Image*> images, Sampler* sampler, VkBuffer vertexBuffer, VkBuffer indexBuffer);
	void updateRaytracingRenderTarget(VkDevice device, VkImageView target);
	void createShaderBindingTable(VkDevice device, VkPhysicalDevice physicalDevice);

private:
	AccelerationStructure m_accelerationStructure;
	
	struct InvMvpUBO
	{
		glm::mat4 viewInverse;
		glm::mat4 projInverse;
	};
	UniformBufferObject m_uboInvMVP;

	struct ParamsUBO
	{
		glm::uint sampleCount;
	};
	UniformBufferObject m_uboParams;
	ParamsUBO m_uboParamsData;

	std::unique_ptr<nv_helpers_vk::DescriptorSetGenerator> m_rtDSG;
	VkDescriptorPool m_rtDescriptorPool;
	VkDescriptorSetLayout m_rtDescriptorSetLayout;
	VkDescriptorSet m_rtDescriptorSet;

	RayTracingPipeline m_pipeline;

	nv_helpers_vk::ShaderBindingTableGenerator m_sbtGen;
	VkBuffer m_shaderBindingTableBuffer;
	VkDeviceMemory m_shaderBindingTableMem;

	Texture m_textureTarget;
	Command m_command;
};

