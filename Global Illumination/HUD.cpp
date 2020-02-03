#include "HUD.h"

#include <utility>

void HUD::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue,
                     VkExtent2D outputExtent)
{
	m_outputExtent = outputExtent;
	m_font.initialize(device, physicalDevice, commandPool, graphicsQueue, 48, "Fonts/arial.ttf");

	buildFPSCounter(device, physicalDevice, commandPool, graphicsQueue, L"FPS : 0");

	m_attachments.resize(2);
	m_attachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	m_attachments[1].initialize(VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	m_renderPass.initialize(device, physicalDevice, commandPool, m_attachments, { outputExtent });

	std::vector<Texture*> textures = m_font.getTextures();

	std::vector<TextureLayout> textureLayouts(textures.size());
	for (int i(0); i < textureLayouts.size(); ++i)
	{
		textureLayouts[i].accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
		textureLayouts[i].binding = i + 1;
	}

	UniformBufferObjectLayout uboLayout;
	uboLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
	uboLayout.binding = 0;

	m_renderer.initialize(device, "Shaders/hud/textVert.spv", "Shaders/hud/textFrag.spv", { Vertex2DTexturedWithMaterial::getBindingDescription(0) }, Vertex2DTexturedWithMaterial::getAttributeDescriptions(0), 
		{ uboLayout }, textureLayouts, { true });

	const VertexBuffer fpsCounterVertexBuffer = m_fpsCounter.getVertexBuffer();
	std::vector<std::pair<Texture*, TextureLayout>> rendererTextures(textures.size());
	for (int j(0); j < rendererTextures.size(); ++j)
	{
		rendererTextures[j].first = textures[j];
		rendererTextures[j].second = textureLayouts[j];
	}
	
	m_fpsCounterIDRenderer = m_renderer.addMesh(device, descriptorPool, fpsCounterVertexBuffer, { { m_fpsCounter.getUBO(), uboLayout } }, rendererTextures);

	m_menu.initialize(device, physicalDevice, commandPool, graphicsQueue, outputExtent, "Fonts/arial.ttf", std::function<void(void*, VkImageView)>(), this);
	m_menu.addBooleanItem(device, physicalDevice, commandPool, graphicsQueue, L"Draw FPS Counter", std::function<void(void*, bool)>(), true, this, { "", "" });
	m_menu.build(device, physicalDevice, commandPool, descriptorPool, graphicsQueue);

	fillCommandBuffer(device, m_currentDrawMenu);
}

void HUD::submit(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, int fps, bool drawMenu)
{
	if(fps > 0)
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
	
	m_renderPass.submit(device, graphicsQueue, 0, {});
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
	renderers.push_back(&m_renderer);
	m_renderPass.fillCommandBuffer(device, 0, m_clearValues, renderers);
}
