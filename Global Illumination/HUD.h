#pragma once

#include "Text.h"
#include "RenderPass.h"
#include "Menu.h"

class HUD
{
public:
	HUD() = default;
	~HUD() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkExtent2D outputExtent);
	void submit(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, int fps, bool drawMenu);

	Semaphore* getRenderCompleteSemaphore() { return m_renderPass.getRenderCompleteSemaphore(); }
	std::vector<Image*> getImages() { return m_renderPass.getImages(0); }

private:
	void buildFPSCounter(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring textValue);
	void fillCommandBuffer(VkDevice device, bool drawMenu);

private:
	std::vector<Attachment> m_attachments;
	std::vector<VkClearValue> m_clearValues;
	RenderPass m_renderPass;

	VkExtent2D m_outputExtent;
	
	Font m_font;
	Text m_fpsCounter;
	int m_fpsCounterIDRenderer;

	Menu m_menu;
	bool m_currentDrawMenu = false;

	Renderer m_renderer;
};
