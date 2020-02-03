#pragma once

#include "VulkanHelper.h"
#include "Pipeline.h"
#include "vulkannv/nv_helpers_vk/RaytracingPipelineGenerator.h"

class RayTracingPipeline
{
public:
	RayTracingPipeline() = default;
	~RayTracingPipeline() = default;

	void initialize(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, std::string raygenShaderFilename, std::string missShaderFilename, std::string closestHitShaderFilename);
	void cleanup(VkDevice device);

	uint32_t getRayGenIndex() const { return m_rayGenIndex; }
	uint32_t getHitGroupIndex() const { return m_hitGroupIndex; }
	uint32_t getMissIndex() const {	return m_missIndex; }
	VkPipeline getPipeline() const { return m_pipeline; }
	VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }

	uint32_t getShadowHitGroupIndex() { return  m_shadowHitGroupIndex; }
	uint32_t getShadowMissIndex() { return m_shadowMissIndex; }

private:
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;

	uint32_t m_rayGenIndex{};
	uint32_t m_hitGroupIndex{};
	uint32_t m_missIndex{};
	
	uint32_t m_shadowMissIndex;
	uint32_t m_shadowHitGroupIndex;
};

