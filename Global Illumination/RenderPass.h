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
	std::vector<VkImageView> imageViews; // added after the mesh image views
};

struct Semaphore
{
    VkSemaphore semaphore;
    VkPipelineStageFlags stage;
};

class RenderPass
{
public:
	~RenderPass();

	void initialize(Vulkan* vk, std::vector<VkExtent2D> extent, bool present, VkSampleCountFlagBits msaaSamples, bool colorAttachment = true, bool depthAttachment = true);

	int addMesh(Vulkan * vk, std::vector<MeshRender> mesh, std::string vertPath, std::string fragPath, int nbTexture, bool alphaBlending = false, int frameBufferID = 0);
	int addMeshInstanced(Vulkan* vk, std::vector<MeshRender> meshes, std::string vertPath, std::string fragPath, int nbTexture);
	int addText(Vulkan * vk, Text * text);
	void addMenu(Vulkan* vk, Menu * menu);
	void updateImageViewMenuItemOption(Vulkan* vk, VkImageView imageView);
	void recordDraw(Vulkan * vk);

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
	void createRenderPass(VkDevice device, VkImageLayout finalLayout, bool colorAttachment, bool depthAttachment);
	void createColorResources(Vulkan * vk, VkExtent2D extent);
	VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device, std::vector<UboBase*> uniformBuffers, int nbTexture);
	void createDescriptorPool(VkDevice device);
	VkDescriptorSet createDescriptorSet(VkDevice device, VkDescriptorSetLayout decriptorSetLayout, std::vector<VkImageView> imageView,
		VkSampler sampler, std::vector<UboBase*> uniformBuffers, int nbTexture, std::vector<VkImageLayout> imageLayouts);
	void fillCommandBuffer(Vulkan * vk);
	void drawFrame(Vulkan * vk);

public:
	VkRenderPass getRenderPass() { return m_renderPass; }
	FrameBuffer getFrameBuffer(int index) { return m_frameBuffers[index]; }
	VkSemaphore getRenderFinishedSemaphore() { return m_renderCompleteSemaphore; }

private:
	bool m_isInitialized = false;

	VkFormat m_format;
	VkFormat m_depthFormat;
	bool m_useColorAttachment = false;
	bool m_useDepthAttachment = false;

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

