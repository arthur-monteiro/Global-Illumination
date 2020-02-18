#pragma once

#include "Text.h"
#include "RenderPass.h"
#include "Menu.h"

class HUD
{
public:
	HUD() = default;
	~HUD() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkExtent2D outputExtent,
		std::function<void(void*, std::string, std::wstring)> callback, void* instance, bool raytracingAvailable);
	void submit(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, GLFWwindow* window, int fps, bool drawMenu);

	void resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkExtent2D outputExtent);
	void cleanup(VkDevice device, VkCommandPool commandPool);

	Semaphore* getRenderCompleteSemaphore() { return m_renderPass.getRenderCompleteSemaphore(); }
	std::vector<Image*> getImages() { return m_renderPass.getImages(0); }

	static void drawFPSCounterCallback(void* instance, bool status) { reinterpret_cast<HUD*>(instance)->drawFPSCounter(status); }
	static void changeShadowsCallback(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("shadow", option); }
	static void changeUpscaleCallback(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("upscale", option); }
	static void changeMSAACallback(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("msaa", option); }
	static void changeRTShadowsAA(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("rtshadow_sample_count", option); }
	static void changeAO(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("ao", option); }
	static void changeSSAOPower(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("ssao_power", option); }
	static void changeReflections(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("reflection", option); }

	void drawFPSCounter(bool status);
	void applyCallback(std::string parameter, std::wstring value);

private:
	void buildMenu(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue);
	void buildFPSCounter(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring textValue);
	void fillCommandBuffer(VkDevice device, bool drawMenu);

private:	
	std::vector<Attachment> m_attachments;
	std::vector<VkClearValue> m_clearValues;
	RenderPass m_renderPass;
	bool m_rayTracingAvailable = false;

	VkExtent2D m_outputExtent;
	
	Font m_font;
	Text m_fpsCounter;
	int m_fpsCounterIDRenderer;
	bool m_drawFPSCounter = true;

	Menu m_menu;
	bool m_currentDrawMenu = false;
	int m_needToDisable = -1;
	int m_needToEnable = -1;
	int m_msaaItem, m_upscaleItem;

	Renderer m_renderer;
	bool m_shouldRefillCommandBuffer = false;

	std::function<void(void*, std::string, std::wstring)> m_callback;
	void* m_instanceForCallback;
};
