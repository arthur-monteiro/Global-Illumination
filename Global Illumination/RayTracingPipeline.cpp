#include "RayTracingPipeline.h"

void RayTracingPipeline::initialize(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, std::string raygenShaderFilename, std::string missShaderFilename, std::string closestHitShaderFilename)
{
	nv_helpers_vk::RayTracingPipelineGenerator pipelineGen;

	VkShaderModule rayGenModule = Pipeline::createShaderModule(Pipeline::readFile(raygenShaderFilename), device);
	m_rayGenIndex = pipelineGen.AddRayGenShaderStage(rayGenModule);

	VkShaderModule missModule = Pipeline::createShaderModule(Pipeline::readFile(missShaderFilename), device);
	m_missIndex = pipelineGen.AddMissShaderStage(missModule);

	VkShaderModule missShadowModule = Pipeline::createShaderModule(Pipeline::readFile("Shaders/rtShadows/shadowRMiss.spv"), device);
	m_shadowMissIndex = pipelineGen.AddMissShaderStage(missShadowModule);

	m_hitGroupIndex = pipelineGen.StartHitGroup();
	VkShaderModule closestHitModule = Pipeline::createShaderModule(Pipeline::readFile(closestHitShaderFilename), device);
	pipelineGen.AddHitShaderStage(closestHitModule, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
	pipelineGen.EndHitGroup();

	// The second hit group defines the shaders invoked when a shadow ray hits the
	// geometry. For simple shadows we do not need any shader in that group: we will rely on
	// initializing the payload and update it only in the miss shader
	m_shadowHitGroupIndex = pipelineGen.StartHitGroup();
	pipelineGen.EndHitGroup();

	pipelineGen.SetMaxRecursionDepth(2);

	// Generate the pipeline
	pipelineGen.Generate(device, descriptorSetLayout, &m_pipeline,
		&m_pipelineLayout);

	vkDestroyShaderModule(device, rayGenModule, nullptr);
	vkDestroyShaderModule(device, missModule, nullptr);
	vkDestroyShaderModule(device, closestHitModule, nullptr);
	vkDestroyShaderModule(device, missShadowModule, nullptr);
}

void RayTracingPipeline::cleanup(VkDevice device)
{
	vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
	vkDestroyPipeline(device, m_pipeline, nullptr);
}
