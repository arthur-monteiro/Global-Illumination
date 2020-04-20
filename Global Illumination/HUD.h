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
		std::function<void(void*, std::string, std::wstring)> callback, std::function<void(void*, std::string, std::vector<float>)> floatCallback, void* instance, HardwareCapabilities hardwareCapabilities);
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
	static void changeCSMShadowsAA(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("csm_soft_option", option); }
	static void changeCSMSSIterations(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("csm_ss_iterations", option); }
	static void changeCSMSSFactor(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("csm_ss_factor", option); }
	static void changeCSMBlurAmount(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("csm_blur_amount", option); }
	static void changeAO(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("ao", option); }
	static void changeSSAOPower(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("ssao_power", option); }
	static void changeBloom(void* instance, bool status) { reinterpret_cast<HUD*>(instance)->applyCallback("bloom", status ? L"true" : L"false"); }
	static void changeReflections(void* instance, std::wstring option) { reinterpret_cast<HUD*>(instance)->applyCallback("reflection", option); }
	static void changeMotionBlur(void* instance, bool status) { reinterpret_cast<HUD*>(instance)->applyCallback("motion_blur", status ? L"true" : L"false"); }
	static void changeDepthOfField(void* instance, bool status) { reinterpret_cast<HUD*>(instance)->applyCallback("depth_of_field", status ? L"true" : L"false"); }
	static void changeBokeh(void* instance, bool status) { reinterpret_cast<HUD*>(instance)->applyCallback("bokeh", status ? L"true" : L"false"); }
	static void changeBokehThresholdBrightness(void* instance, std::vector<float> values) { reinterpret_cast<HUD*>(instance)->applyFloatCallback("bokeh_threshold_brightness", std::move(values)); }
	static void changeBokehThresholdBlur(void* instance, std::vector<float> values) { reinterpret_cast<HUD*>(instance)->applyFloatCallback("bokeh_threshold_blur", std::move(values)); }
	static void changeFocusPointCentered(void* instance, bool status) { reinterpret_cast<HUD*>(instance)->applyCallback("bokeh_focus_point_centered", status ? L"true" : L"false"); }
	static void changeBlurFocus(void* instance, std::vector<float> values) { reinterpret_cast<HUD*>(instance)->applyFloatCallback("bokeh_focus_point", std::move(values)); }
	static void changeExposure(void* instance, std::vector<float> values) { reinterpret_cast<HUD*>(instance)->applyFloatCallback("exposure", std::move(values)); }
	static void changeGamma(void* instance, std::vector<float> values) { reinterpret_cast<HUD*>(instance)->applyFloatCallback("gamma", std::move(values)); }

	void drawFPSCounter(bool status);
	void applyCallback(std::string parameter, std::wstring value);
	void applyFloatCallback(std::string parameter, std::vector<float> values);

private:
	void buildMenu(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue);
	void buildFPSCounter(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring textValue);
	void fillCommandBuffer(VkDevice device, bool drawMenu);
	static int getIndexInVector(const std::vector<std::wstring>& vector, const std::wstring& value);

private:
	std::vector<Attachment> m_attachments;
	std::vector<VkClearValue> m_clearValues;
	RenderPass m_renderPass;
	HardwareCapabilities m_hardwareCapabilities;

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

	// Menu current state
	std::wstring m_shadowState = L"No";
	std::wstring m_upscaleState = L"No";
	std::wstring m_mssaState = L"No";
	std::wstring m_rtShadowAAState = L"No";
	std::wstring m_csmShadowAAState = L"No";
	std::wstring m_aoState = L"No";
	std::wstring m_ssaoPowerState = L"1";
	bool m_bloomState = false;
	std::wstring m_reflectionState = L"No";
	std::wstring m_csmSSIterations = L"8";
	std::wstring m_csmSSFactor = L"1 / 700";
	std::wstring m_csmBlurAmount = L"No";
	bool m_motionBlurState = false;
	bool m_depthOfFieldState = false;
	bool m_bokehState = false;
	float m_bokehThresholdBrightnessState = 3.5f;
	float m_bokehThresholdBlurState = 0.5f;
	bool m_focusPointCenteredState = true;
	std::vector<float> m_blurFocusState = { 0.0f, 4.0f, 10.0f, 16.0f };
	float m_exposureState = 0.5f;
	float m_gammaState = 2.2f;

	Renderer m_renderer;
	bool m_shouldRefillCommandBuffer = false;

	std::function<void(void*, std::string, std::wstring)> m_callback;
	std::function<void(void*, std::string, std::vector<float>)> m_floatCallback;
	void* m_instanceForCallback;
};
