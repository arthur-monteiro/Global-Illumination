#pragma once

#include "ComputePass.h"
#include "RenderPass.h"
#include "Blur.h"

class DepthOfField
{
public:
	DepthOfField() = default;
	~DepthOfField() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool computeCommandPool, VkCommandPool graphicsCommandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue, VkQueue graphicsQueue,
		Texture* inputViewPos, Texture* inputColor);
	void submit(VkDevice device, VkQueue computeQueue, VkQueue graphicsQueue, std::vector<Semaphore*> semaphoresToWait);
	void cleanup(VkDevice device, VkCommandPool computeCommandPool, VkCommandPool graphicsCommandPool, VkDescriptorPool descriptorPool);

	void setBokehActivation(bool activate);
	void setBokehThresholdBrightness(float brightnessThreshold);
	void setBokehThresholdBlur(float blurThreshold);
	void setCenterFocusPoint(bool centered);
	void setFocusPoint(glm::vec4 focusPoint);

	Texture* getOutputTexture() { return m_drawBokehPoints || m_useBlur ? &m_outputTextureMerge : &m_outputPoissonBlurTexture[1]; }
	Semaphore* getSemaphore() { return m_drawBokehPoints || m_useBlur ? &m_mergeSemaphore : &m_poissonBlurSemaphore[1]; }
	bool isReady() const { return m_isReady; }

private:
	bool m_isReady = false;
	
	/* -- First pass -> compute circle of confusion size and copy depth -- */
	ComputePass m_depthBlurPass;
	Texture m_outputDepthBlurTexture;
	Semaphore m_depthBlurSemaphore;

	glm::vec4 m_oldFocusPoint = glm::vec4(0.0f, 4.0f, 10.0f, 24.0f);
	struct DepthBlurUBO
	{
		glm::vec4 focusPoint = glm::vec4(-1.0f);
	};
	DepthBlurUBO m_depthBlurUBOData;
	UniformBufferObject m_depthBlurUBO;
	bool m_needUpdateDepthBlurUBO = false;

	/* Second pass -> Populate a buffer with bokeh points -- */
	bool m_drawBokehPoints = false;
	struct BokehPoint
	{
		glm::vec4 position;
		glm::vec4 blurAndColor;
	};

	struct BokehPointGenerationParams
	{
		glm::vec4 params;
	};
	BokehPointGenerationParams m_bokehPointGenerationParams;
	
	RenderPass m_bokehPointGenerationRenderPass;
	std::vector<Attachment> m_bokehPointGenerationAttachements;
	std::vector<VkClearValue> m_bokehPointGenerationClearValues;

	VkBuffer m_bokehPointsBuffer;
	VkDeviceMemory m_bokehPointsBufferMemory;

	UniformBufferObject m_bokehPointsGenerationUBO;
	bool m_needUpdateBokehPointsGenerationUBO = false;

	// The current amount of bokeh points is stored in a indirect command buffer
	VkDrawIndexedIndirectCommand m_indirectBokehPointsDrawCommand;
	VkBuffer m_indirectBokehPointsDrawCommandBufferDefault; // this buffer act as staging buffer but we keep it to reset the actual buffer after each frame
	VkDeviceMemory m_indirectBokehPointsDrawCommandBufferMemoryDefault;
	VkBuffer m_indirectBokehPointsDrawCommandBuffer;
	VkDeviceMemory m_indirectBokehPointsDrawCommandBufferMemory;

	Mesh<Vertex2DTextured> m_quad;

	Renderer m_bokehPointGenerationRenderer;

	/* -- Third pass -> Draw bokeh shapes --*/
	RenderPass m_bokedPointDrawRenderPass;
	Texture m_bokehPointDrawRenderPassResultTexture;
	std::vector<Attachment> m_bokedPointDrawAttachments;
	std::vector<VkClearValue> m_bokedPointDrawClearValues;

	Pipeline m_bokehPointDrawPipeline;
	VkDescriptorSetLayout m_bokehPointDrawDescriptorSetLayout;
	VkDescriptorSet m_bokehPointDrawDescriptorSet;
	VkCommandBuffer m_bokehPointDrawCommandBuffer;

	Semaphore m_bokehPointDrawSemaphore;

	Texture m_bokehShapeTexture;

	Blur m_bokehPointDrawBlur;

	// Blur then interpolate between blurred version and non blurred version
	/* -- Blur -- */
	bool m_useBlur = false;
	Blur m_blur;
	Texture m_textureBlur;

	// Blur with a poisson disk
	bool m_usePoissonDiskSampling = true;
	std::array<ComputePass, 2> m_poissonBlur;
	std::array<Texture, 2> m_outputPoissonBlurTexture;
	std::array<Semaphore, 2> m_poissonBlurSemaphore;

	/* -- Merge -- */
	ComputePass m_merge;
	Texture m_outputTextureMerge;
	Semaphore m_mergeSemaphore;

	// TEMP
	/*struct PosAndColor
	{
		glm::vec4 pos;
		glm::vec4 color;
	};
	std::vector<PosAndColor> m_posAndColorData;
	VkBuffer m_bufferWithSquarePositions;
	VkDeviceMemory m_bufferWithSquarePositionsMemory;
	RenderPass m_renderPass;
	std::vector<Attachment> m_attachments;
	Pipeline m_pipeline;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkCommandBuffer m_commandBuffer;
	std::vector<VkClearValue> m_clearValues;
	Semaphore m_tempSemaphore;
	VkDrawIndexedIndirectCommand m_indirectDrawCommand;
	VkBuffer m_indirectDrawCommandBuffer;
	VkDeviceMemory m_indirectDrawCommandBufferMemory;
	RenderPass m_renderPassFillVertices;
	std::vector<Attachment> m_attachmenRenderPassFillVertices;
	Pipeline m_pipelineFillVertices;
	VkDescriptorSetLayout m_descriptorSetLayoutFillVertices;
	VkCommandBuffer m_commandBufferFillVertices;
	Semaphore m_tempSemaphoreFillVertices;
	VkDescriptorSet m_descriptorSetFillVertices;
	VkBuffer drawCmdIndirectBufferDefault;
	VkDeviceMemory drawCmdIndirectBufferDefaultMemory;*/
};

