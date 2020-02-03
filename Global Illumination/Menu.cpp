#include "Menu.h"

void Menu::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D outputExtent, std::string fontPath,
	std::function<void(void*, VkImageView)> callbackSetImageView, void* instance)
{
	m_callbackSetImageView = callbackSetImageView;
	m_callerInstance = instance;
	m_outputExtent = outputExtent;

	m_font.initialize(device, physicalDevice, commandPool, graphicsQueue, 48, fontPath);
}

int Menu::addBooleanItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, bool)> callback,
	bool defaultValue, void* instance, std::array<std::string, 2> imageOptions)
{
	m_booleanItems.push_back(BooleanItem());
	m_booleanItems[m_booleanItems.size() - 1].value = defaultValue;
	m_booleanItems[m_booleanItems.size() - 1].callback = callback;
	m_booleanItems[m_booleanItems.size() - 1].instance = instance;
	
	m_booleanItems[m_booleanItems.size() - 1].textID = m_text.addWString(label, glm::vec2(ITEM_X_OFFSET + TEXT_X_OFFSET,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), glm::vec3(1.0f));

	m_booleanItems[m_booleanItems.size() - 1].textNoID = m_text.addWString(L"No", glm::vec2(ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(L"Yes  /  No", m_outputExtent, &m_font, TEXT_SIZE) / 2.0f,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), defaultValue ? TEXT_VALUE_COLOR_NO : TEXT_VALUE_COLOR_YES);
	m_booleanItems[m_booleanItems.size() - 1].textSeparatorID = m_text.addWString(L"/", glm::vec2(
		ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(L"Yes  /  No", m_outputExtent, &m_font, TEXT_SIZE) / 2.0f + m_text.simulateSizeX(L"No  ", m_outputExtent, &m_font, TEXT_SIZE),
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), TEXT_VALUE_COLOR_NO);
	m_booleanItems[m_booleanItems.size() - 1].textYesID = m_text.addWString(L"Yes", glm::vec2(
		ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(L"Yes  /  No", m_outputExtent, &m_font, TEXT_SIZE) / 2.0f + m_text.simulateSizeX(L"No  /  ", m_outputExtent, &m_font, TEXT_SIZE),
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), defaultValue ? TEXT_VALUE_COLOR_YES : TEXT_VALUE_COLOR_NO);
	
	m_booleanItems[m_booleanItems.size() - 1].quadID = addQuadItem(MENU_ITEM_TYPE_BOOLEAN, m_booleanItems.size() - 1);

	m_numberOfItems++;

	for (int i(0); i < 2; ++i)
	{
		if (imageOptions[i] != "")
		{
			m_booleanItems[m_booleanItems.size() - 1].texturesOptions[i].createFromFile(device, physicalDevice, commandPool, graphicsQueue, imageOptions[i]);
			//m_currentOptionImageView = m_booleanItems[m_booleanItems.size() - 1].imageOptions[i].getImageView();
		}
	}

	return m_booleanItems.size() - 1;
}

void Menu::build(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue)
{
	// Texts
	m_text.build(device, physicalDevice, commandPool, graphicsQueue, m_outputExtent, &m_font, TEXT_SIZE);

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

	m_textRenderer.initialize(device, "Shaders/hud/textVert.spv", "Shaders/hud/textFrag.spv", { Vertex2DTexturedWithMaterial::getBindingDescription(0) }, Vertex2DTexturedWithMaterial::getAttributeDescriptions(0),
		{ uboLayout }, textureLayouts, { true });
	
	const VertexBuffer fpsCounterVertexBuffer = m_text.getVertexBuffer();
	std::vector<std::pair<Texture*, TextureLayout>> rendererTextures(textures.size());
	for (int j(0); j < rendererTextures.size(); ++j)
	{
		rendererTextures[j].first = textures[j];
		rendererTextures[j].second = textureLayouts[j];
	}

	m_textRenderer.addMesh(device, descriptorPool, fpsCounterVertexBuffer, { { m_text.getUBO(), uboLayout } }, rendererTextures);

	// Quads
	std::vector<VkVertexInputAttributeDescription> quadVertexAttributeDescriptions = Vertex2D::getAttributeDescriptions(0);
	std::vector<VkVertexInputAttributeDescription> instanceAttributeDescriptions = InstanceSingleID::getAttributeDescriptions(1, 1);
	for(const VkVertexInputAttributeDescription& instanceAttributeDescription : instanceAttributeDescriptions)
	{
		quadVertexAttributeDescriptions.push_back(instanceAttributeDescription);
	}

	UniformBufferObjectLayout uboLayoutQuad;
	uboLayoutQuad.accessibility = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutQuad.binding = 0;
	
	m_quadRenderer.initialize(device, "Shaders/hud/quadVert.spv", "Shaders/hud/quadFrag.spv", { Vertex2D::getBindingDescription(0), InstanceSingleID::getBindingDescription(1) },
		quadVertexAttributeDescriptions, { uboLayoutQuad }, {}, { true });

	m_quad.loadFromVertices(device, physicalDevice, commandPool, graphicsQueue, VERTEX_QUAD, INDICES_QUAD);

	std::vector<InstanceSingleID> quadInstances;
	
	for(int i(0); i < m_quadItems.size(); ++i)
	{
		InstanceSingleID quadInstance;
		quadInstance.id = i;

		quadInstances.push_back(quadInstance);

		m_uboQuadsData.transform[i] = m_quadItems[i].transform;
		m_uboQuadsData.color[i] = m_quadItems[i].color;
	}
	m_uboQuads.initialize(device, physicalDevice, &m_uboQuadsData, sizeof(m_uboQuadsData));

	m_quadInstances.loadFromVector(device, physicalDevice, commandPool, graphicsQueue, quadInstances);

	m_quadRenderer.addMeshInstancied(device, descriptorPool, m_quad.getVertexBuffer(), m_quadInstances.getInstanceBuffer(), { { &m_uboQuads, uboLayoutQuad } }, {});
}

void Menu::cleanup(VkDevice device)
{
	m_text.cleanup(device);
}

int Menu::addQuadItem(const int type, const int id)
{
	m_quadItems.emplace_back();
	m_quadItems[m_quadItems.size() - 1].type = type;
	m_quadItems[m_quadItems.size() - 1].structID = id;

	m_quadItems[m_quadItems.size() - 1].color = glm::vec4(ITEM_DEFAULT_COLOR, 1.0f);
	
	// Translate
	const float rangeX = ITEM_X_SIZE / 2.0f;
	const float rangeY = ITEM_Y_SIZE / 2.0f;
	m_quadItems[m_quadItems.size() - 1].transform = glm::translate(glm::mat4(1.0f), glm::vec3(ITEM_X_OFFSET + 1.0f - (1.0f - rangeX), ITEM_Y_OFFSET + 1.0f - (1.0f - rangeY) + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS), 0.0f));
	
	// Scale
	m_quadItems[m_quadItems.size() - 1].transform = glm::scale(m_quadItems[m_quadItems.size() - 1].transform, glm::vec3(ITEM_X_SIZE / 2.0f, ITEM_Y_SIZE / 2.0f, 1.0f));
	
	return m_quadItems.size() - 1;
}
