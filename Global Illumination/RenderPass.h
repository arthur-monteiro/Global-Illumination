#pragma once

#include <array>
#include <chrono>

#include "Vulkan.h"
#include "Pipeline.h"
#include "Mesh.h"
#include "Text.h"
#include "UniformBufferObject.h"
#include "Instance.h"
#include "Menu.h"

struct MeshRender
{
	std::vector<MeshBase*> meshes;
	std::vector<UboBase*> ubos;
	Instance* instance = nullptr;
	std::vector<std::pair<VkImageView, VkImageLayout>> images; // added after the mesh image views
	std::vector<VkSampler> samplers;
};

struct Semaphore
{
    VkSemaphore semaphore;
    VkPipelineStageFlags stage;
};

struct Operation
{
#define OPERATION_TYPE_UNDEFINED -1
#define OPERATION_TYPE_BLIT 0
#define OPERATION_TYPE_COPY 1
#define OPERATION_TYPE_COPY_DEPTH_TO_BUFFER 2
#define OPERATION_TYPE_COPY_BUFFER_TO_IMAGE 3

	int type = OPERATION_TYPE_UNDEFINED;

	// Blit or copy or copy buffer to image
	std::vector<VkImage> dstImages;
	std::vector<VkImage> srcImages;
	// Blit
	std::vector<VkExtent2D> dstBlitExtent;

	// Copy image to buffer
	std::vector<VkBuffer> dstBuffers;

	// Copy buffer to image
	std::vector<VkBuffer> srcBuffers;
};

struct FrameBuffer
{
	std::vector<Image> colorImages;
	Image depthImage;
	std::vector<Image> resolveImages;

	VkFramebuffer framebuffer;

	void free(VkDevice device)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);

		for (int i(0); i < colorImages.size(); ++i)
			colorImages[i].cleanup(device);
		depthImage.cleanup(device);
	}
};

class RenderPass
{
public:
	~RenderPass();

	void initialize(Vulkan* vk, std::vector<VkExtent2D> extent, bool present, VkSampleCountFlagBits msaaSamples, std::vector<VkFormat> colorFormats, VkFormat depthFormat,
		VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	int addMesh(Vulkan * vk, std::vector<MeshRender> mesh, PipelineShaders pipelineShaders, int nbTexture, bool alphaBlending = false, int frameBufferID = 0);
	int addMeshInstanced(Vulkan* vk, std::vector<MeshRender> meshes, std::string vertPath, std::string fragPath, int nbTexture);
	int addText(Vulkan * vk, Text * text);
	void addMenu(Vulkan* vk, Menu * menu);
	void updateImageViewMenuItemOption(Vulkan* vk, VkImageView imageView);
	void clearMeshes(VkDevice device);
	void recordDraw(Vulkan* vk, std::vector<Operation> operations = {});

	void drawCall(Vulkan * vk);

	void cleanup(Vulkan * vk);

	void setDrawMenu(Vulkan * vk, bool draw)
	{ 
		m_drawMenu = draw;
		recordDraw(vk);
	}
	void setDrawText(Vulkan* vk, bool draw)
	{
		m_drawText = draw;
		recordDraw(vk);
	}
	void setSemaphoreToWait(VkDevice device, std::vector<Semaphore> semaphores);
	bool getDrawMenu() { return m_drawMenu; }

private:
	void createRenderPass(VkDevice device, VkImageLayout finalLayout);
	FrameBuffer createFrameBuffer(Vulkan* vk, VkExtent2D extent, VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples, VkFormat depthFormat, std::vector<VkFormat> imageFormats);
	void createColorResources(Vulkan * vk, VkExtent2D extent);
	VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device, std::vector<UboBase*> uniformBuffers, int nbTexture);
	void createDescriptorPool(VkDevice device);
	VkDescriptorSet createDescriptorSet(VkDevice device, VkDescriptorSetLayout decriptorSetLayout, std::vector<VkImageView> imageView,
		std::vector<VkSampler> samplers, std::vector<UboBase*> uniformBuffers, int nbTexture, std::vector<VkImageLayout> imageLayouts);
	void fillCommandBuffer(Vulkan * vk, std::vector<Operation> operations);
	void drawFrame(Vulkan * vk);

public:
	VkRenderPass getRenderPass() { return m_renderPass; }
	FrameBuffer getFrameBuffer(int index) { return m_frameBuffers[index]; }
	VkSemaphore getRenderFinishedSemaphore() { return m_renderCompleteSemaphore; }
	bool getIsInitialized() { return m_isInitialized; }

private:
	bool m_isInitialized = false;

	std::vector<VkFormat> m_colorAttachmentFormats;
	VkFormat m_depthAttachmentFormat;

	VkRenderPass m_renderPass;
	VkDescriptorPool m_descriptorPool;
	
	std::vector<int> m_menuMeshesPipelineIDs;
	bool m_drawMenu = false;
	int m_menuOptionImageMeshPipelineID = -1;
	bool m_drawOptionImage = false;
	VkDescriptorSetLayout m_menuOptionImageDescriptorLayout;
	VkSampler m_menuOptionImageSampler;
	std::vector<MeshPipeline> m_meshesPipeline;

	std::vector<int> m_textID;
	Text * m_text = nullptr;
	std::vector<MeshRender> m_meshes;
	bool m_drawText = true;

	Pipeline m_textPipeline;
	VkDescriptorSetLayout m_textDescriptorSetLayout;

	int m_commandBufferStatic;
	std::vector<int> m_commandBufferDynamics;

	bool m_useSwapChain = true;
	bool m_firstDraw = true;
	std::vector<FrameBuffer> m_frameBuffers;
	std::vector<VkExtent2D> m_extent;
	std::vector <VkCommandBuffer> m_commandBuffer;
	std::vector<VkSemaphore> m_needToWaitSemaphores;
	std::vector<VkPipelineStageFlags> m_needToWaitStages;
	VkSemaphore m_renderCompleteSemaphore;
	VkCommandPool m_commandPool;

	VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage m_colorImage;
	VkDeviceMemory m_colorImageMemory;
	VkImageView m_colorImageView = VK_NULL_HANDLE;
};

