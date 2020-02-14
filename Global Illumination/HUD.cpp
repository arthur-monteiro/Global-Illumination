#include "HUD.h"

#include <utility>

void HUD::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue,
                     VkExtent2D outputExtent, std::function<void(void*, std::string, std::wstring)> callback, void* instance, bool raytracingAvailable)
{
	m_callback = callback;
	m_instanceForCallback = instance;
	m_rayTracingAvailable = raytracingAvailable;
	
	m_outputExtent = outputExtent;
	m_font.initialize(device, physicalDevice, commandPool, graphicsQueue, 48, "Fonts/arial.ttf");

	buildFPSCounter(device, physicalDevice, commandPool, graphicsQueue, L"FPS : 0");

	m_attachments.resize(2);
	m_attachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	m_attachments[1].initialize(VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	m_renderPass.initialize(device, physicalDevice, commandPool, m_attachments, { outputExtent });

	std::vector<Image*> images = m_font.getImages();
	Sampler* sampler = m_font.getSampler();

	SamplerLayout samplerLayout;
	samplerLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayout.binding = 1;

	std::vector<ImageLayout> imageLayouts(images.size());
	for (int i(0); i < imageLayouts.size(); ++i)
	{
		imageLayouts[i].accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
		imageLayouts[i].binding = i + 2;
	}

	UniformBufferObjectLayout uboLayout;
	uboLayout.accessibility = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayout.binding = 0;

	m_renderer.initialize(device, "Shaders/hud/textVert.spv", "Shaders/hud/textFrag.spv", { Vertex2DTexturedWithMaterial::getBindingDescription(0) }, 
		Vertex2DTexturedWithMaterial::getAttributeDescriptions(0), { uboLayout }, {}, imageLayouts, { samplerLayout }, { true });

	const VertexBuffer fpsCounterVertexBuffer = m_fpsCounter.getVertexBuffer();
	std::vector<std::pair<Image*, ImageLayout>> rendererImages(images.size());
	for (int j(0); j < rendererImages.size(); ++j)
	{
		rendererImages[j].first = images[j];
		rendererImages[j].second = imageLayouts[j];
	}
	
	m_fpsCounterIDRenderer = m_renderer.addMesh(device, descriptorPool, fpsCounterVertexBuffer, { { m_fpsCounter.getUBO(), uboLayout } }, {}, rendererImages, 
		{ { sampler, samplerLayout} });

	buildMenu(device, physicalDevice, commandPool, descriptorPool, graphicsQueue);

	fillCommandBuffer(device, m_currentDrawMenu);
}

void HUD::submit(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, GLFWwindow* window, int fps, bool drawMenu)
{
	if(fps > 0 || m_shouldRefillCommandBuffer)
	{
		m_fpsCounter.cleanup(device);
		
		std::wstring text = L"FPS : " + std::to_wstring(fps);
		buildFPSCounter(device, physicalDevice, commandPool, graphicsQueue, text);

		m_renderer.reloadMeshVertexBuffer(device, m_fpsCounter.getVertexBuffer(), m_fpsCounterIDRenderer);
		fillCommandBuffer(device, drawMenu);
	}
	else if(m_currentDrawMenu != drawMenu)
	{
		fillCommandBuffer(device, drawMenu);
		m_currentDrawMenu = drawMenu;
	}

	if (drawMenu)
		m_menu.update(device, window, m_outputExtent.width, m_outputExtent.height);
	
	m_renderPass.submit(device, graphicsQueue, 0, {});
}

void HUD::resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkExtent2D outputExtent)
{
	m_outputExtent = outputExtent;
	
	m_renderPass.cleanup(device, commandPool);
	m_renderPass.initialize(device, physicalDevice, commandPool, m_attachments, { m_outputExtent });

	m_renderer.setPipelineCreated(false);

	m_menu.cleanup(device, descriptorPool);
	buildMenu(device, physicalDevice, commandPool, descriptorPool, graphicsQueue);

	m_shouldRefillCommandBuffer = true;
}

void HUD::cleanup(VkDevice device, VkCommandPool commandPool)
{
	m_renderPass.cleanup(device, commandPool);
	m_font.cleanup(device);
	m_fpsCounter.cleanup(device);
}

void HUD::drawFPSCounter(bool status)
{
	m_drawFPSCounter = status;
	m_shouldRefillCommandBuffer = true;
}

void HUD::buildMenu(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue)
{
	m_menu.initialize(device, physicalDevice, commandPool, graphicsQueue, m_outputExtent, std::function<void(void*, VkImageView)>(), this);
	m_menu.addBooleanItem(device, physicalDevice, commandPool, graphicsQueue, L"Draw FPS Counter", drawFPSCounterCallback, true, this, 
		{ "", "" }, &m_font);
	m_menu.addPicklistItem(device, physicalDevice, commandPool, graphicsQueue, L"Upscale", changeUpscaleCallback, this, 0,
		{ L"No", L"2x", L"4x", L"8x" }, &m_font);
	m_menu.addPicklistItem(device, physicalDevice, commandPool, graphicsQueue, L"MSAA", changeMSAACallback, this, 0,
		{ L"No", L"2x", L"4x", L"8x" }, &m_font);
	
	std::vector<std::wstring> shadowOptions = { L"No" };
	if (m_rayTracingAvailable)
		shadowOptions.push_back(L"NVidia Ray Tracing");
	int shadowMenuItem = m_menu.addPicklistItem(device, physicalDevice, commandPool, graphicsQueue, L"Shadows", changeShadowsCallback, this, 0, 
		shadowOptions, &m_font);

	int rtShadowAAItem = m_menu.addDependentPicklistItem(device, physicalDevice, commandPool, graphicsQueue, L"Shadow Anti-aliasing", changeRTShadowsAA, this, 0,
		{ L"No", L"2x", L"4x", L"8x" }, &m_font, MENU_ITEM_TYPE_PICKLIST, shadowMenuItem, { 1 });
	int aoItem = m_menu.addPicklistItem(device, physicalDevice, commandPool, graphicsQueue, L"Ambient Occlusion", changeAO, this, 0,
		{ L"No", L"SSAO" }, &m_font);
	m_menu.addDependentPicklistItem(device, physicalDevice, commandPool, graphicsQueue, L"Power", changeSSAOPower, this, 0,
		{ L"1", L"2", L"5", L"10", L"100" }, &m_font, MENU_ITEM_TYPE_PICKLIST, aoItem, { 1 });
	m_menu.build(device, physicalDevice, commandPool, descriptorPool, graphicsQueue, &m_font);
}

void HUD::buildFPSCounter(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring textValue)
{
	m_fpsCounter.addWString(std::move(textValue), glm::vec2(-0.99f, 0.85f), glm::vec3(1.0f));
	m_fpsCounter.build(device, physicalDevice, commandPool, graphicsQueue, m_outputExtent, &m_font, 0.065f);
}

void HUD::fillCommandBuffer(VkDevice device, bool drawMenu)
{
	m_clearValues.resize(2);
	m_clearValues[0] = { 1.0f };
	if(drawMenu)
		m_clearValues[1] = { 0.0f, 0.0f, 0.0f, 0.6f };
	else
		m_clearValues[1] = { 0.0f, 0.0f, 0.0f, 0.0f };

	std::vector<Renderer*> renderers;
	if(drawMenu)
	{
		renderers = m_menu.getRenderers();
	}
	if(m_drawFPSCounter)
		renderers.push_back(&m_renderer);
	
	m_renderPass.fillCommandBuffer(device, 0, m_clearValues, renderers);
	m_shouldRefillCommandBuffer = false;
}
