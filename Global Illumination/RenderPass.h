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
	MeshBase* mesh;
	std::vector<UboBase*> ubos;
	Instance* instance = nullptr;
	std::vector<VkImageView> imageViews; // added after the mesh image views
};

class RenderPass
{
public:
	~RenderPass();

	void initialize(Vulkan* vk, bool createFrameBuffer = false, VkExtent2D extent = { 0, 0 }, bool present = true, VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT, int nbFramebuffer = 1, 
		bool colorAttachment = true, bool depthAttachment = true);

	int addMesh(Vulkan * vk, std::vector<MeshRender> mesh, std::string vertPath, std::string fragPath, int nbTexture, bool alphaBlending = false, int frameBufferID = 0);
	int addMeshInstanced(Vulkan* vk, std::vector<MeshRender> meshes, std::string vertPath, std::string fragPath, int nbTexture);
	int addText(Vulkan * vk, Text * text);
	void addMenu(Vulkan* vk, Menu * menu);

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
	bool getDrawMenu() { return m_drawMenu; }

private:
	void createRenderPass(VkDevice device, VkImageLayout finalLayout, bool colorAttachment, bool depthAttachment);
	void createColorResources(Vulkan * vk, VkExtent2D extent);
	VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device, std::vector<UboBase*> uniformBuffers, int nbTexture);
	void createDescriptorPool(VkDevice device);
	VkDescriptorSet createDescriptorSet(VkDevice device, VkDescriptorSetLayout decriptorSetLayout, std::vector<VkImageView> imageView,
		VkSampler sampler, std::vector<UboBase*> uniformBuffers, int nbTexture, VkImageLayout imageLayout);
	void fillCommandBuffer(Vulkan * vk);
	void drawFrame(Vulkan * vk);

public:
	VkRenderPass getRenderPass() { return m_renderPass; }
	FrameBuffer getFrameBuffer(int index) { return m_frameBuffers[index]; }
	VkSemaphore getRenderFinishedSemaphore() { return m_renderCompleteSemaphore; }

private:
	bool m_isDestroyed = false;

	VkFormat m_format;
	VkFormat m_depthFormat;

	VkRenderPass m_renderPass;
	VkDescriptorPool m_descriptorPool;
	
	std::vector<int> m_menuMeshesPipelineIDs;
	bool m_drawMenu = false;
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
	VkExtent2D m_extent;
	std::vector <VkCommandBuffer> m_commandBuffer;
	VkSemaphore m_renderCompleteSemaphore;
	VkCommandPool m_commandPool;

	VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage m_colorImage;
	VkDeviceMemory m_colorImageMemory;
	VkImageView m_colorImageView = VK_NULL_HANDLE;
};

