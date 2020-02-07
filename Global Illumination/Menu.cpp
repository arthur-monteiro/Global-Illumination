#include "Menu.h"

void Menu::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D outputExtent,
	std::function<void(void*, VkImageView)> callbackSetImageView, void* instance)
{
	m_callbackSetImageView = callbackSetImageView;
	m_callerInstance = instance;
	m_outputExtent = outputExtent;

	//m_font.initialize(device, physicalDevice, commandPool, graphicsQueue, 48, fontPath);
}

int Menu::addBooleanItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, bool)> callback,
	bool defaultValue, void* instance, std::array<std::string, 2> imageOptions, Font* font)
{
	m_booleanItems.push_back(BooleanItem());
	m_booleanItems[m_booleanItems.size() - 1].value = defaultValue;
	m_booleanItems[m_booleanItems.size() - 1].callback = callback;
	m_booleanItems[m_booleanItems.size() - 1].instance = instance;
	
	m_booleanItems[m_booleanItems.size() - 1].textID = m_text.addWString(label, glm::vec2(ITEM_X_OFFSET + TEXT_X_OFFSET,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), glm::vec3(1.0f));

	m_booleanItems[m_booleanItems.size() - 1].textNoID = m_text.addWString(L"No", glm::vec2(ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(L"Yes  /  No", m_outputExtent, font, TEXT_SIZE) / 2.0f,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), defaultValue ? TEXT_VALUE_COLOR_NO : TEXT_VALUE_COLOR_YES);
	m_booleanItems[m_booleanItems.size() - 1].textSeparatorID = m_text.addWString(L"/", glm::vec2(
		ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(L"Yes  /  No", m_outputExtent, font, TEXT_SIZE) / 2.0f + m_text.simulateSizeX(L"No  ", m_outputExtent, font, TEXT_SIZE),
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), TEXT_VALUE_COLOR_NO);
	m_booleanItems[m_booleanItems.size() - 1].textYesID = m_text.addWString(L"Yes", glm::vec2(
		ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(L"Yes  /  No", m_outputExtent, font, TEXT_SIZE) / 2.0f + m_text.simulateSizeX(L"No  /  ", m_outputExtent, font, TEXT_SIZE),
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

int Menu::addPicklistItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkQueue graphicsQueue, std::wstring label, std::function<void(void*, std::wstring)> callback, void* instance,
	int defaultValue, std::vector<std::wstring> options, Font* font, glm::vec2 offset)
{
	m_picklistItems.push_back(PicklistItem());
	m_picklistItems[m_picklistItems.size() - 1].selectedOption = defaultValue;
	m_picklistItems[m_picklistItems.size() - 1].options = options;
	m_picklistItems[m_picklistItems.size() - 1].callback = callback;
	m_picklistItems[m_picklistItems.size() - 1].instance = instance;
	m_picklistItems[m_picklistItems.size() - 1].textID = m_text.addWString(label, glm::vec2(ITEM_X_OFFSET + TEXT_X_OFFSET,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f) + offset, glm::vec3(1.0f));
	m_picklistItems[m_picklistItems.size() - 1].quadID = addQuadItem(MENU_ITEM_TYPE_PICKLIST, m_picklistItems.size() - 1, offset);

	m_picklistItems[m_picklistItems.size() - 1].textAngleBrackets[0] = m_text.addWString( L">", 
		glm::vec2(ITEM_X_OFFSET + ITEM_X_SIZE - 0.035f - m_text.simulateSizeX(L">", m_outputExtent, font, TEXT_SIZE) / 2.0f,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f) + offset, glm::vec3(0.7f));
	m_picklistItems[m_picklistItems.size() - 1].textAngleBrackets[1] = m_text.addWString(L"<", 
		glm::vec2(ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE + 0.035f - m_text.simulateSizeX(L"<", m_outputExtent, font, TEXT_SIZE) / 2.0f,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f) + offset, glm::vec3(0.7f));

	for (int i(0); i < options.size(); ++i)
	{
		m_picklistItems[m_picklistItems.size() - 1].textOptionIDs.push_back(m_text.addWString(options[i], 
			glm::vec2(ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(options[i], m_outputExtent, font, TEXT_SIZE) / 2.0f,
			ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f) + offset,
			i == defaultValue ? TEXT_VALUE_COLOR_YES : glm::vec3(-1.0f)));
	}

	m_numberOfItems++;
	return m_picklistItems.size() - 1;
}

int Menu::addDependentPicklistItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                                   VkQueue graphicsQueue, std::wstring label, std::function<void(void*, std::wstring)> callback, void* instance,
                                   int defaultValue, std::vector<std::wstring> options, Font* font, int itemTypeSrc, int itemIdSrc, std::vector<int> activateValues)
{
	int r =  addPicklistItem(device, physicalDevice, commandPool, graphicsQueue, std::move(label), std::move(callback), instance, defaultValue,
	                       std::move(options), font, glm::vec2(DEPENDENT_ITEM_X_OFFSET, 0.0f));

	m_picklistItems[r].masterItemType = itemTypeSrc;
	m_picklistItems[r].masterItemID = itemIdSrc;
	m_picklistItems[r].masterValuesActivation = activateValues;

	if (itemTypeSrc == MENU_ITEM_TYPE_PICKLIST)
	{
		/*m_picklistItems[itemIdSrc].valueToActivateItem = activateValues;
		m_picklistItems[itemIdSrc].activateItemType = MENU_ITEM_TYPE_PICKLIST;
		m_picklistItems[itemIdSrc].activateItemID = r;*/
	}

	return r;
}

void Menu::build(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, Font* font)
{
	// Texts
	m_text.build(device, physicalDevice, commandPool, graphicsQueue, m_outputExtent, font, TEXT_SIZE);

	std::vector<Texture*> textures = font->getTextures();

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
	}
	m_uboQuads.initialize(device, physicalDevice, &m_uboQuadsData, sizeof(m_uboQuadsData));
	updateQuadUBO(device);

	m_quadInstances.loadFromVector(device, physicalDevice, commandPool, graphicsQueue, quadInstances);

	m_quadRenderer.addMeshInstancied(device, descriptorPool, m_quad.getVertexBuffer(), m_quadInstances.getInstanceBuffer(), { { &m_uboQuads, uboLayoutQuad } }, {});

	// Check elements to hide
	for(int i(0); i < m_picklistItems.size(); ++i)
	{
		if(m_picklistItems[i].masterItemID >= 0)
		{
			int selectedOptionByMasterItem = -1;
			if (m_picklistItems[i].masterItemType == MENU_ITEM_TYPE_PICKLIST)
				selectedOptionByMasterItem = m_picklistItems[m_picklistItems[i].masterItemID].selectedOption;

			if (std::find(m_picklistItems[i].masterValuesActivation.begin(), m_picklistItems[i].masterValuesActivation.end(), selectedOptionByMasterItem) == 
				m_picklistItems[i].masterValuesActivation.end())
			{
				hidePicklistItem(device, i);
			}
		}
	}
}

void Menu::update(VkDevice device, GLFWwindow* window, int windowWidth, int windowHeight)
{
	double mousePosX, mousePosY;
	glfwGetCursorPos(window, &mousePosX, &mousePosY);
	// [0, 1]
	mousePosX /= windowWidth;
	mousePosY /= windowHeight;
	// [0, 2]
	mousePosX *= 2.0;
	mousePosY *= 2.0;
	// [-1, 1]
	mousePosX -= 1.0;
	mousePosY -= 1.0;

	bool needUpdateUBO = false;
	for (int i(0); i < m_quadItems.size(); ++i) // check for mouse in quad
	{
		if (m_quadItems[i].type == MENU_ITEM_TYPE_BOOLEAN && !m_booleanItems[m_quadItems[i].structID].activated)
			continue;
		if(m_quadItems[i].type == MENU_ITEM_TYPE_PICKLIST && !m_picklistItems[m_quadItems[i].structID].activated)
			continue;

		bool focus = false;
		if (mousePosX > m_quadItems[i].posOffset.x + ITEM_X_OFFSET && mousePosX < m_quadItems[i].posOffset.x + (ITEM_X_OFFSET + ITEM_X_SIZE) &&
			mousePosY > ITEM_Y_OFFSET + i * (SPACE_BETWEEN_ITEMS + ITEM_Y_SIZE) + m_quadItems[i].posOffset.y 
			&& mousePosY < ITEM_Y_OFFSET + i * (SPACE_BETWEEN_ITEMS + ITEM_Y_SIZE) + ITEM_Y_SIZE + m_quadItems[i].posOffset.y)
			focus = true;

		if (setFocus(i, focus))
			needUpdateUBO = true;
	}
	if(needUpdateUBO)
		updateQuadUBO(device);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && m_oldMouseLeftState == GLFW_RELEASE) // left click
	{
		for (int i(0); i < m_quadItems.size(); ++i)
		{
			if (m_quadItems[i].onFocus)
			{
				switch (m_quadItems[i].type)
				{
				case MENU_ITEM_TYPE_BOOLEAN:
					onClickBooleanItem(device, m_quadItems[i].structID);
					break;
				case MENU_ITEM_TYPE_PICKLIST:
					onClickPicklistItem(device, m_quadItems[i].structID);
					break;
				case MENU_ITEM_TYPE_UNDEFINED:
					break;
				default:
					break;
				}
			}
		}
	}

	m_oldMouseLeftState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);
}

void Menu::cleanup(VkDevice device, VkDescriptorPool descriptorPool)
{
	//m_font.cleanup(device);
	m_text.cleanup(device);

	m_quad.cleanup(device);
	m_quadInstances.cleanup(device);
	//m_quadImageOption.cleanup(device);

	m_numberOfItems = 0;
	m_booleanItems.clear();
	m_quadItems.clear();

	m_quadRenderer.cleanup(device, descriptorPool);
	m_textRenderer.cleanup(device, descriptorPool);

	m_uboQuads.cleanup(device);
}

int Menu::addQuadItem(const int type, const int id, glm::vec2 offset)
{
	m_quadItems.emplace_back();
	m_quadItems[m_quadItems.size() - 1].type = type;
	m_quadItems[m_quadItems.size() - 1].structID = id;

	m_quadItems[m_quadItems.size() - 1].color = glm::vec4(ITEM_DEFAULT_COLOR, 1.0f);
	
	// Translate
	const float rangeX = ITEM_X_SIZE / 2.0f;
	const float rangeY = ITEM_Y_SIZE / 2.0f;
	m_quadItems[m_quadItems.size() - 1].transform = glm::translate(glm::mat4(1.0f), 
		glm::vec3(ITEM_X_OFFSET + 1.0f - (1.0f - rangeX), ITEM_Y_OFFSET + 1.0f - (1.0f - rangeY) + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS), 0.0f) 
		+ glm::vec3(offset, 0.0f));
	
	// Scale
	m_quadItems[m_quadItems.size() - 1].transform = glm::scale(m_quadItems[m_quadItems.size() - 1].transform, glm::vec3(ITEM_X_SIZE / 2.0f, ITEM_Y_SIZE / 2.0f, 1.0f));

	m_quadItems[m_quadItems.size() - 1].posOffset = offset;
	
	return m_quadItems.size() - 1;
}

bool Menu::setFocus(int id, bool focus)
{
	if (m_quadItems[id].onFocus != focus)
	{
		m_quadItems[id].onFocus = focus;
		m_quadItems[id].color = glm::vec4(focus ? ITEM_FOCUS_COLOR : ITEM_DEFAULT_COLOR, 1.0f);
		m_quadItems[id].transform = glm::scale(m_quadItems[id].transform, focus ? glm::vec3(1.05f, 1.05f, 1.0f) : glm::vec3(0.95238f, 0.95238f, 1.0f));

		return true;
	}

	return false;
}

void Menu::updateQuadUBO(VkDevice device)
{
	for (int i(0); i < m_quadItems.size(); ++i)
	{
		m_uboQuadsData.transform[i] = m_quadItems[i].transform;
		m_uboQuadsData.color[i] = m_quadItems[i].color;
	}
	m_uboQuads.updateData(device, &m_uboQuadsData);
}

void Menu::onClickBooleanItem(VkDevice device, int id)
{
	m_booleanItems[id].value = m_booleanItems[id].value ? false : true;

	m_text.setColor(device, m_booleanItems[id].textYesID, m_booleanItems[id].value ? TEXT_VALUE_COLOR_YES : TEXT_VALUE_COLOR_NO);
	m_text.setColor(device, m_booleanItems[id].textNoID, m_booleanItems[id].value ? TEXT_VALUE_COLOR_NO : TEXT_VALUE_COLOR_YES);

	//m_currentOptionImageView = m_booleanItems[id].imageOptions[m_booleanItems[id].value ? 1 : 0].getImageView();

	m_booleanItems[id].callback(m_booleanItems[id].instance, m_booleanItems[id].value);
}

void Menu::onClickPicklistItem(VkDevice device, int id)
{
	m_text.setColor(device, m_picklistItems[id].textOptionIDs[m_picklistItems[id].selectedOption], glm::vec3(-1.0f));
	m_picklistItems[id].selectedOption = (m_picklistItems[id].selectedOption + 1) % m_picklistItems[id].options.size();
	m_text.setColor(device, m_picklistItems[id].textOptionIDs[m_picklistItems[id].selectedOption], TEXT_VALUE_COLOR_YES);

	m_picklistItems[id].callback(m_picklistItems[id].instance, m_picklistItems[id].options[m_picklistItems[id].selectedOption]);

	// Check for dependent picklist items
	for(int i(0); i < m_picklistItems.size(); ++i)
	{
		if(m_picklistItems[i].masterItemType == MENU_ITEM_TYPE_PICKLIST && m_picklistItems[i].masterItemID == id)
		{
			const bool shouldBeActivated = std::find(m_picklistItems[i].masterValuesActivation.begin(), m_picklistItems[i].masterValuesActivation.end(), m_picklistItems[id].selectedOption) !=
				m_picklistItems[i].masterValuesActivation.end();

			if (shouldBeActivated && !m_picklistItems[i].activated)
				showPicklistItem(device, i);
			else if (!shouldBeActivated && m_picklistItems[i].activated)
				hidePicklistItem(device, i);
		}
	}
}

void Menu::hidePicklistItem(VkDevice device, int id)
{
	m_text.setColor(device, m_picklistItems[id].textID, glm::vec3(-1.0f));
	for(int& text : m_picklistItems[id].textOptionIDs)
		m_text.setColor(device, text, glm::vec3(-1.0f));
	for(int& text : m_picklistItems[id].textAngleBrackets)
		m_text.setColor(device, text, glm::vec3(-1.0f));

	m_quadItems[m_picklistItems[id].quadID].color = glm::vec4(-1.0f);
	updateQuadUBO(device);

	m_picklistItems[id].activated = false;
}

void Menu::showPicklistItem(VkDevice device, int id)
{
	m_text.setColor(device, m_picklistItems[id].textID, glm::vec3(1.0f));
	for (int& text : m_picklistItems[id].textAngleBrackets)
		m_text.setColor(device, text, glm::vec3(0.7f));
	for (int i(0); i < m_picklistItems[id].textOptionIDs.size(); ++i)
	{
		m_text.setColor(device, m_picklistItems[id].textOptionIDs[i], m_picklistItems[id].selectedOption == i ? TEXT_VALUE_COLOR_YES : glm::vec3(-1.0f));
	}

	m_quadItems[m_picklistItems[id].quadID].color = glm::vec4(ITEM_DEFAULT_COLOR, 1.0f);
	updateQuadUBO(device);

	m_picklistItems[id].activated = true;
}
