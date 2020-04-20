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
	bool defaultValue, void* instance, std::array<std::string, 2> imageOptions, Font* font, glm::vec2 offset)
{
	m_booleanItems.push_back(BooleanItem());
	m_booleanItems[m_booleanItems.size() - 1].value = defaultValue;
	m_booleanItems[m_booleanItems.size() - 1].callback = callback;
	m_booleanItems[m_booleanItems.size() - 1].instance = instance;

	m_booleanItems[m_booleanItems.size() - 1].textID = m_text.addWString(label, glm::vec2(ITEM_X_OFFSET + TEXT_X_OFFSET,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f) + offset, glm::vec3(1.0f));

	m_booleanItems[m_booleanItems.size() - 1].textNoID = m_text.addWString(L"No", glm::vec2(ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(L"Yes  /  No", m_outputExtent, font, TEXT_SIZE) / 2.0f,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f) + offset, defaultValue ? TEXT_VALUE_COLOR_NO : TEXT_VALUE_COLOR_YES);
	m_booleanItems[m_booleanItems.size() - 1].textSeparatorID = m_text.addWString(L"/", glm::vec2(
		ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(L"Yes  /  No", m_outputExtent, font, TEXT_SIZE) / 2.0f + m_text.simulateSizeX(L"No  ", m_outputExtent, font, TEXT_SIZE),
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f) + offset, TEXT_VALUE_COLOR_NO);
	m_booleanItems[m_booleanItems.size() - 1].textYesID = m_text.addWString(L"Yes", glm::vec2(
		ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(L"Yes  /  No", m_outputExtent, font, TEXT_SIZE) / 2.0f + m_text.simulateSizeX(L"No  /  ", m_outputExtent, font, TEXT_SIZE),
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f) + offset, defaultValue ? TEXT_VALUE_COLOR_YES : TEXT_VALUE_COLOR_NO);

	m_booleanItems[m_booleanItems.size() - 1].quadID = addQuadItem(MENU_ITEM_TYPE_BOOLEAN, m_booleanItems.size() - 1, offset);

	m_numberOfItems++;

	for (int i(0); i < 2; ++i)
	{
		if (imageOptions[i] != "")
		{
			m_booleanItems[m_booleanItems.size() - 1].texturesOptions[i].createFromFile(device, physicalDevice, commandPool, graphicsQueue, imageOptions[i]);
			m_currentOptionImageView = m_booleanItems[m_booleanItems.size() - 1].texturesOptions[i].getImageView();
		}
	}

	return m_booleanItems.size() - 1;
}

int Menu::addDependentBooleanItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkQueue graphicsQueue, std::wstring label, std::function<void(void*, bool)> callback, bool defaultValue,
	void* instance, std::array<std::string, 2> imageOptions, Font* font, int itemTypeSrc, int itemIdSrc,
	std::vector<int> activateValues)
{
	int r = addBooleanItem(device, physicalDevice, commandPool, graphicsQueue, std::move(label), std::move(callback), defaultValue, instance,
		std::move(imageOptions), font, glm::vec2(DEPENDENT_ITEM_X_OFFSET, 0.0f));

	m_booleanItems[r].masterItemType = itemTypeSrc;
	m_booleanItems[r].masterItemID = itemIdSrc;
	m_booleanItems[r].masterValuesActivation = activateValues;

	return r;
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

	m_picklistItems[m_picklistItems.size() - 1].textAngleBrackets[0] = m_text.addWString(L">",
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
	int r = addPicklistItem(device, physicalDevice, commandPool, graphicsQueue, std::move(label), std::move(callback), instance, defaultValue,
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

int Menu::addRangeSliderItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkQueue graphicsQueue, std::wstring label, std::function<void(void*, std::vector<float>)> callback, void* instance,
	std::vector<float> defaultValues, float rangeStart, float rangeEnd, Font* font, glm::vec2 offset)
{
	m_rangeSliderItems.push_back(RangeSliderItem());
	m_rangeSliderItems[m_rangeSliderItems.size() - 1].start = rangeStart;
	m_rangeSliderItems[m_rangeSliderItems.size() - 1].end = rangeEnd;
	m_rangeSliderItems[m_rangeSliderItems.size() - 1].values = defaultValues;
	m_rangeSliderItems[m_rangeSliderItems.size() - 1].callback = std::move(callback);
	m_rangeSliderItems[m_rangeSliderItems.size() - 1].instance = instance;
	m_rangeSliderItems[m_rangeSliderItems.size() - 1].textID = m_text.addWString(std::move(label), glm::vec2(ITEM_X_OFFSET + TEXT_X_OFFSET,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f) + offset, glm::vec3(1.0f));
	m_rangeSliderItems[m_rangeSliderItems.size() - 1].quadID = addQuadItem(MENU_ITEM_TYPE_RANGE_SLIDER, m_rangeSliderItems.size() - 1, offset);

	m_quadItems.emplace_back();
	m_quadItems[m_quadItems.size() - 1].type = MENU_ITEM_TYPE_UNDEFINED;
	m_quadItems[m_quadItems.size() - 1].structID = -1;
	m_quadItems[m_quadItems.size() - 1].color = glm::vec4(RANGE_SLIDER_COLOR, 1.0f);
	m_rangeSliderItems[m_rangeSliderItems.size() - 1].rangeBarQuadID = m_quadItems.size() - 1;

	m_rangeSliderItems[m_rangeSliderItems.size() - 1].sliderBarQuadIDs.resize(defaultValues.size());
	for(int i(0); i < defaultValues.size(); ++i)
	{
		m_quadItems.emplace_back();
		m_quadItems[m_quadItems.size() - 1].type = MENU_ITEM_TYPE_UNDEFINED;
		m_quadItems[m_quadItems.size() - 1].structID = -1;
		m_quadItems[m_quadItems.size() - 1].color = glm::vec4(RANGE_SLIDER_ITEM_BAR_COLOR_DEFAULT, 1.0f);
		m_rangeSliderItems[m_rangeSliderItems.size() - 1].sliderBarQuadIDs[i] = m_quadItems.size() - 1;
	}

	updateRangeSliderPosition(m_rangeSliderItems.size() - 1, true);
	
	m_numberOfItems++;
	return m_rangeSliderItems.size() - 1;
}

int Menu::addDependentRangeSliderItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkQueue graphicsQueue, std::wstring label, std::function<void(void*, std::vector<float>)> callback, void* instance,
	std::vector<float> defaultValues, float rangeStart, float rangeEnd, Font* font, int itemTypeSrc, int itemIdSrc,
	std::vector<int> activateValues)
{
	int r = addRangeSliderItem(device, physicalDevice, commandPool, graphicsQueue, std::move(label), std::move(callback), instance, defaultValues,
		rangeStart, rangeEnd, font, glm::vec2(DEPENDENT_ITEM_X_OFFSET, 0.0f));

	m_rangeSliderItems[r].masterItemType = itemTypeSrc;
	m_rangeSliderItems[r].masterItemID = itemIdSrc;
	m_rangeSliderItems[r].masterValuesActivation = std::move(activateValues);

	return r;
}

void Menu::updateRangeSliderPosition(int id, bool changeColor)
{
	int yID = 0;
	for(int i(0); i < m_quadItems.size(); ++i)
	{
		if (i == m_rangeSliderItems[id].quadID)
			break;
		if (m_quadItems[i].type != MENU_ITEM_TYPE_UNDEFINED)
			yID++;
	}
	
	// Range bar
	m_quadItems[m_rangeSliderItems[id].rangeBarQuadID].position = glm::vec2(ITEM_X_OFFSET + RANGE_SLIDER_OFFSET_LEFT + (ITEM_X_SIZE - RANGE_SLIDER_OFFSET_LEFT) / 2.0f,
		ITEM_Y_OFFSET - ITEM_Y_SIZE / 2.0f + yID * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + ITEM_Y_SIZE);
	m_quadItems[m_rangeSliderItems[id].rangeBarQuadID].position -= glm::vec2(m_scrollTranslation.x, m_scrollTranslation.y);
	m_quadItems[m_rangeSliderItems[id].rangeBarQuadID].position += m_quadItems[m_rangeSliderItems[id].quadID].posOffset;

	m_quadItems[m_rangeSliderItems[id].rangeBarQuadID].size = glm::vec2(ITEM_X_SIZE - RANGE_SLIDER_OFFSET_LEFT - RANGE_SLIDER_OFFSET_RIGHT, RANGE_SLIDER_Y_SIZE);

	m_quadItems[m_rangeSliderItems[id].rangeBarQuadID].createTransformMatrix();

	if(changeColor)
		m_quadItems[m_rangeSliderItems[id].rangeBarQuadID].color = glm::vec4(RANGE_SLIDER_COLOR, 1.0f);

	// Sliders
	for(int i(0); i < m_rangeSliderItems[id].sliderBarQuadIDs.size(); ++i)
	{
		float rangeSize = ITEM_X_SIZE - RANGE_SLIDER_OFFSET_LEFT - RANGE_SLIDER_OFFSET_RIGHT;
		float xOffset = (m_rangeSliderItems[id].values[i] - m_rangeSliderItems[id].start) / (m_rangeSliderItems[id].end - m_rangeSliderItems[id].start) * rangeSize;

		m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].position = glm::vec2(ITEM_X_OFFSET + RANGE_SLIDER_OFFSET_LEFT + RANGE_SLIDER_OFFSET_RIGHT / 2.0f + RANGE_SLIDER_ITEM_BAR_X_SIZE / 2.0f + xOffset,
			ITEM_Y_OFFSET - ITEM_Y_SIZE / 2.0f + yID * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + ITEM_Y_SIZE);
		m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].position += m_quadItems[m_rangeSliderItems[id].quadID].posOffset;
		m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].position -= glm::vec2(m_scrollTranslation.x, m_scrollTranslation.y);

		m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].size = glm::vec2(RANGE_SLIDER_ITEM_BAR_X_SIZE, RANGE_SLIDER_ITEM_BAR_Y_SIZE);

		m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].createTransformMatrix();

		if(changeColor)
			m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].color = glm::vec4(RANGE_SLIDER_ITEM_BAR_COLOR_DEFAULT, 1.0f);
	}
}

void Menu::disableItem(VkDevice device, int itemType, int itemId)
{
	if (itemType == MENU_ITEM_TYPE_PICKLIST)
	{
		m_picklistItems[itemId].activated = false;
		m_quadItems[m_picklistItems[itemId].quadID].color = glm::vec4(ITEM_DEACTIVATED_COLOR, 1.0f);

		m_text.setColor(device, m_picklistItems[itemId].textID, TEXT_COLOR_DEACTIVATED);
		m_text.setColor(device, m_picklistItems[itemId].textOptionIDs[m_picklistItems[itemId].selectedOption], TEXT_COLOR_DEACTIVATED);
		for (int& text : m_picklistItems[itemId].textAngleBrackets)
			m_text.setColor(device, text, TEXT_COLOR_DEACTIVATED);
	}

	updateQuadUBO(device);
}

void Menu::enableItem(VkDevice device, int itemType, int itemId)
{
	if (itemType == MENU_ITEM_TYPE_PICKLIST)
	{
		m_picklistItems[itemId].activated = true;
		m_quadItems[m_picklistItems[itemId].quadID].color = glm::vec4(ITEM_DEFAULT_COLOR, 1.0f);

		m_text.setColor(device, m_picklistItems[itemId].textID, TEXT_VALUE_COLOR_YES);
		m_text.setColor(device, m_picklistItems[itemId].textOptionIDs[m_picklistItems[itemId].selectedOption], TEXT_VALUE_COLOR_YES);
		for (int& text : m_picklistItems[itemId].textAngleBrackets)
			m_text.setColor(device, text, glm::vec3(0.7f));
	}

	updateQuadUBO(device);
}

void Menu::build(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, Font* font)
{
	// Texts
	m_text.build(device, physicalDevice, commandPool, graphicsQueue, m_outputExtent, font, TEXT_SIZE);

	std::vector<Image*> images = font->getImages();
	Sampler* sampler = font->getSampler();

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

	m_textRenderer.initialize(device, "Shaders/hud/textMenuItemsVert.spv", "Shaders/hud/textMenuItemsFrag.spv", { Vertex2DTexturedWithMaterial::getBindingDescription(0) },
		Vertex2DTexturedWithMaterial::getAttributeDescriptions(0), { uboLayout }, {}, imageLayouts, { samplerLayout }, {},  { true });

	const VertexBuffer textVertexBuffer = m_text.getVertexBuffer();
	std::vector<std::pair<Image*, ImageLayout>> rendererImages(images.size());
	for (int j(0); j < rendererImages.size(); ++j)
	{
		rendererImages[j].first = images[j];
		rendererImages[j].second = imageLayouts[j];
	}

	m_textRenderer.addMesh(device, descriptorPool, textVertexBuffer, { { m_text.getUBO(), uboLayout } }, {}, rendererImages,
		{ { sampler, samplerLayout} }, {});

	// Quads
	std::vector<VkVertexInputAttributeDescription> quadVertexAttributeDescriptions = Vertex2D::getAttributeDescriptions(0);
	std::vector<VkVertexInputAttributeDescription> instanceAttributeDescriptions = InstanceSingleID::getAttributeDescriptions(1, 1);
	for (const VkVertexInputAttributeDescription& instanceAttributeDescription : instanceAttributeDescriptions)
	{
		quadVertexAttributeDescriptions.push_back(instanceAttributeDescription);
	}

	UniformBufferObjectLayout uboLayoutQuad;
	uboLayoutQuad.accessibility = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutQuad.binding = 0;

	m_quadRenderer.initialize(device, "Shaders/hud/quadVert.spv", "Shaders/hud/quadFrag.spv", { Vertex2D::getBindingDescription(0), InstanceSingleID::getBindingDescription(1) },
		quadVertexAttributeDescriptions, { uboLayoutQuad }, {}, {}, {}, {}, { true });

	m_quad.loadFromVertices(device, physicalDevice, commandPool, graphicsQueue, VERTEX_QUAD, INDICES_QUAD);

	std::vector<InstanceSingleID> quadInstances;

	// Scroll bar
	{
		m_quadItems.emplace_back();
		m_quadItems[m_quadItems.size() - 1].type = MENU_ITEM_TYPE_UNDEFINED;
		m_quadItems[m_quadItems.size() - 1].structID = -1;

		m_quadItems[m_quadItems.size() - 1].color = glm::vec4(SCROOL_BAR_DEFAULT_COLOR, 1.0f);

		m_scrollBarQuadItemID = m_quadItems.size() - 1;
	}

	for (int i(0); i < m_quadItems.size(); ++i)
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
	for (int i(0); i < m_booleanItems.size(); ++i)
	{
		if (m_booleanItems[i].masterItemID >= 0)
		{
			int selectedOptionByMasterItem = -1;
			if (m_booleanItems[i].masterItemType == MENU_ITEM_TYPE_BOOLEAN)
				selectedOptionByMasterItem = m_booleanItems[m_booleanItems[i].masterItemID].value ? 1 : 0;

			if (m_booleanItems[i].masterValuesActivation[0] != selectedOptionByMasterItem)
			{
				hideBooleanItem(device, i);
			}
		}
	}
	for (int i(0); i < m_picklistItems.size(); ++i)
	{
		if (m_picklistItems[i].masterItemID >= 0)
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
	for(int i(0); i < m_rangeSliderItems.size(); ++i)
	{
		if(m_rangeSliderItems[i].masterItemID >= 0)
		{
			int selectedOptionByMasterItem = -1;
			if (m_rangeSliderItems[i].masterItemType == MENU_ITEM_TYPE_BOOLEAN)
				selectedOptionByMasterItem = m_booleanItems[m_rangeSliderItems[i].masterItemID].value ? 1 : 0;

			if (m_rangeSliderItems[i].masterValuesActivation[0] != selectedOptionByMasterItem)
			{
				hideRangeSliderItem(device, i);
			}
		}
	}

	updateScrollBar(); // udpate depends on which element are hidden
	updateQuadUBO(device);
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
	int idQuad = -1;
	for (int i(0); i < m_quadItems.size(); ++i) // check for mouse in quad
	{
		if (m_quadItems[i].type == MENU_ITEM_TYPE_UNDEFINED)
			continue;
		else
			idQuad++;
		
		if (m_quadItems[i].type == MENU_ITEM_TYPE_BOOLEAN && !m_booleanItems[m_quadItems[i].structID].activated)
			continue;
		if (m_quadItems[i].type == MENU_ITEM_TYPE_PICKLIST && !m_picklistItems[m_quadItems[i].structID].activated)
			continue;
		if (m_quadItems[i].type == MENU_ITEM_TYPE_RANGE_SLIDER && !m_rangeSliderItems[m_quadItems[i].structID].activated)
			continue;

		bool focus = false;
		if (mousePosX > m_quadItems[i].posOffset.x + ITEM_X_OFFSET && mousePosX < m_quadItems[i].posOffset.x + (ITEM_X_OFFSET + ITEM_X_SIZE) &&
			mousePosY > ITEM_Y_OFFSET + idQuad * (SPACE_BETWEEN_ITEMS + ITEM_Y_SIZE) + m_quadItems[i].posOffset.y - m_scrollTranslation.y
			&& mousePosY < ITEM_Y_OFFSET + idQuad * (SPACE_BETWEEN_ITEMS + ITEM_Y_SIZE) + ITEM_Y_SIZE + m_quadItems[i].posOffset.y - m_scrollTranslation.y)
			focus = true;

		if (setFocus(i, focus))
			needUpdateUBO = true;
	}
	// Check for mouse in scroll bar
	bool scrollBarOnFocus = false;
	{
		float scrollBarStartX = ITEM_X_OFFSET + ITEM_X_SIZE + DEPENDENT_ITEM_X_OFFSET + SCROLL_BAR_X_OFFSET - SCROLL_BAR_X_SIZE;
		if (mousePosX > scrollBarStartX&& mousePosX < scrollBarStartX + SCROLL_BAR_X_SIZE * 2.0f
			&& mousePosY > m_scrollBarStartY&& mousePosY < m_scrollBarStartY + m_scroolBarYSize)
			scrollBarOnFocus = true;
		if (m_quadItems[m_scrollBarQuadItemID].onFocus != scrollBarOnFocus)
			needUpdateUBO = true;
		m_quadItems[m_scrollBarQuadItemID].onFocus = scrollBarOnFocus;
	}

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
				case MENU_ITEM_TYPE_RANGE_SLIDER:
					onClickRangeSliderItem(device, m_quadItems[i].structID, mousePosX, mousePosY);
					needUpdateUBO = true;
					break;
				case MENU_ITEM_TYPE_UNDEFINED:
					break;
				default:
					break;
				}
			}
		}
		if (scrollBarOnFocus)
		{
			m_isScrolling = true;
			m_previousMousePosForScrolling = glm::vec2(mousePosX, mousePosY);
		}
	}
	else if (m_isScrolling && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		const float scrollYOffset = mousePosY - m_previousMousePosForScrolling.y;
		m_previousMousePosForScrolling = glm::vec2(mousePosX, mousePosY);

		m_quadItems[m_scrollBarQuadItemID].posOffset.y += scrollYOffset;
		if (m_quadItems[m_scrollBarQuadItemID].posOffset.y < 0.0f) m_quadItems[m_scrollBarQuadItemID].posOffset.y = 0.0f;
		else if (m_quadItems[m_scrollBarQuadItemID].posOffset.y > (DRAW_RANGE_Y[1] - DRAW_RANGE_Y[0]) - m_scroolBarYSize)
			m_quadItems[m_scrollBarQuadItemID].posOffset.y = (DRAW_RANGE_Y[1] - DRAW_RANGE_Y[0]) - m_scroolBarYSize;

		setTranformMatrixToAllQuadItems(device);

		needUpdateUBO = true;
	}
	else if (m_isScrolling)
	{
		needUpdateUBO = true;
		m_isScrolling = false;
	}
	else if(m_rangeSliderMovingID >= 0 && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		const float scrollXOffset = mousePosX - m_previousMousePosForScrolling.x;
		m_previousMousePosForScrolling = glm::vec2(mousePosX, mousePosY);

		m_quadItems[m_rangeSliderItems[m_rangeSliderMovingID].sliderBarQuadIDs[m_rangeSliderMovingSlideBarNum]].position.x += scrollXOffset;

		float minX = ITEM_X_OFFSET + RANGE_SLIDER_OFFSET_LEFT + RANGE_SLIDER_OFFSET_RIGHT / 2.0f + m_quadItems[m_rangeSliderItems[m_rangeSliderMovingID].quadID].posOffset.x;
		float maxX = ITEM_X_OFFSET + ITEM_X_SIZE - RANGE_SLIDER_OFFSET_RIGHT / 2.0f + m_quadItems[m_rangeSliderItems[m_rangeSliderMovingID].quadID].posOffset.x;
		if (m_quadItems[m_rangeSliderItems[m_rangeSliderMovingID].sliderBarQuadIDs[m_rangeSliderMovingSlideBarNum]].position.x < minX)
			m_quadItems[m_rangeSliderItems[m_rangeSliderMovingID].sliderBarQuadIDs[m_rangeSliderMovingSlideBarNum]].position.x = minX;
		else if (m_quadItems[m_rangeSliderItems[m_rangeSliderMovingID].sliderBarQuadIDs[m_rangeSliderMovingSlideBarNum]].position.x > maxX)
			m_quadItems[m_rangeSliderItems[m_rangeSliderMovingID].sliderBarQuadIDs[m_rangeSliderMovingSlideBarNum]].position.x = maxX;

		m_quadItems[m_rangeSliderItems[m_rangeSliderMovingID].sliderBarQuadIDs[m_rangeSliderMovingSlideBarNum]].createTransformMatrix();

		m_rangeSliderItems[m_rangeSliderMovingID].values[m_rangeSliderMovingSlideBarNum] = (m_quadItems[m_rangeSliderItems[m_rangeSliderMovingID].sliderBarQuadIDs[m_rangeSliderMovingSlideBarNum]].position.x - minX) / (maxX - minX); // [0; 1]
		m_rangeSliderItems[m_rangeSliderMovingID].values[m_rangeSliderMovingSlideBarNum] *= m_rangeSliderItems[m_rangeSliderMovingID].end - m_rangeSliderItems[m_rangeSliderMovingID].start; // [0; range]
		m_rangeSliderItems[m_rangeSliderMovingID].values[m_rangeSliderMovingSlideBarNum] += m_rangeSliderItems[m_rangeSliderMovingID].start; // [start; end]

		m_rangeSliderItems[m_rangeSliderMovingID].callback(m_rangeSliderItems[m_rangeSliderMovingID].instance, m_rangeSliderItems[m_rangeSliderMovingID].values);

		needUpdateUBO = true;
	}
	else if(m_rangeSliderMovingID >= 0)
	{
		m_quadItems[m_rangeSliderItems[m_rangeSliderMovingID].sliderBarQuadIDs[m_rangeSliderMovingSlideBarNum]].color = glm::vec4(RANGE_SLIDER_ITEM_BAR_COLOR_DEFAULT, 1.0f);
		m_rangeSliderMovingID = -1;
		m_rangeSliderMovingSlideBarNum = -1;

		needUpdateUBO = true;
	}

	if (needUpdateUBO)
	{
		updateScrollBar();
		updateQuadUBO(device);
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
	m_picklistItems.clear();
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

	for (int i(0); i < m_rangeSliderItems.size(); ++i)
	{
		if (m_rangeSliderItems[i].masterItemType == MENU_ITEM_TYPE_BOOLEAN && m_rangeSliderItems[i].masterItemID == id)
		{
			const bool shouldBeActivated = m_rangeSliderItems[i].masterValuesActivation[0] == m_booleanItems[id].value ? 1 : 0;

			if (shouldBeActivated && !m_rangeSliderItems[i].activated)
				showRangeSliderItem(device, i);
			else if (!shouldBeActivated && m_rangeSliderItems[i].activated)
				hideRangeSliderItem(device, i);
		}
	}
	for (int i(0); i < m_booleanItems.size(); ++i)
	{
		if (m_booleanItems[i].masterItemType == MENU_ITEM_TYPE_BOOLEAN && m_booleanItems[i].masterItemID == id)
		{
			const bool shouldBeActivated = m_booleanItems[i].masterValuesActivation[0] == m_booleanItems[id].value ? 1 : 0;

			if (shouldBeActivated && !m_booleanItems[i].activated)
				showBooleanItem(device, i);
			else if (!shouldBeActivated && m_booleanItems[i].activated)
				hideBooleanItem(device, i);
		}
	}
}

void Menu::onClickPicklistItem(VkDevice device, int id)
{
	m_text.setColor(device, m_picklistItems[id].textOptionIDs[m_picklistItems[id].selectedOption], glm::vec3(-1.0f));
	m_picklistItems[id].selectedOption = (m_picklistItems[id].selectedOption + 1) % m_picklistItems[id].options.size();
	m_text.setColor(device, m_picklistItems[id].textOptionIDs[m_picklistItems[id].selectedOption], TEXT_VALUE_COLOR_YES);

	m_picklistItems[id].callback(m_picklistItems[id].instance, m_picklistItems[id].options[m_picklistItems[id].selectedOption]);

	// Check for dependent picklist items
	for (int i(0); i < m_picklistItems.size(); ++i)
	{
		if (m_picklistItems[i].masterItemType == MENU_ITEM_TYPE_PICKLIST && m_picklistItems[i].masterItemID == id)
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

void Menu::onClickRangeSliderItem(VkDevice device, int id, double posX, double posY)
{
	for(int i(0); i < m_rangeSliderItems[id].sliderBarQuadIDs.size(); ++i)
	{
		glm::vec2 topLeft = m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].position - m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].size / 2.0f;
		glm::vec2 botRight = m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].position + m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].size / 2.0f;

		if(posX > topLeft.x && posY > topLeft.y && posX < botRight.x && posY < botRight.y)
		{
			m_quadItems[m_rangeSliderItems[id].sliderBarQuadIDs[i]].color = glm::vec4(RANGE_SLIDER_ITEM_BAR_COLOR_ON_CLICK, 1.0f);
			m_rangeSliderMovingID = id;
			m_rangeSliderMovingSlideBarNum = i;
			m_previousMousePosForScrolling = glm::vec2(posX, posY);
		}
	}
}

void Menu::hideBooleanItem(VkDevice device, int id)
{
	if (!m_booleanItems[id].activated)
		return;

	m_text.setColor(device, m_booleanItems[id].textID, glm::vec3(-1.0f));
	m_text.setColor(device, m_booleanItems[id].textYesID, glm::vec3(-1.0f));
	m_text.setColor(device, m_booleanItems[id].textSeparatorID, glm::vec3(-1.0f));
	m_text.setColor(device, m_booleanItems[id].textNoID, glm::vec3(-1.0f));
	m_quadItems[m_booleanItems[id].quadID].color = glm::vec4(-1.0f);

	addOffsetOnQuads(device, m_booleanItems[id].quadID, -1.0f);

	updateQuadUBO(device);

	m_booleanItems[id].activated = false;

	for (int i(0); i < m_rangeSliderItems.size(); ++i)
	{
		if (m_rangeSliderItems[i].masterItemType == MENU_ITEM_TYPE_BOOLEAN && m_rangeSliderItems[i].masterItemID == id && m_rangeSliderItems[i].activated)
		{
			hideRangeSliderItem(device, i);
		}
	}
}

void Menu::hidePicklistItem(VkDevice device, int id)
{
	if (!m_picklistItems[id].activated)
		return;
	
	m_text.setColor(device, m_picklistItems[id].textID, glm::vec3(-1.0f));
	for (int& text : m_picklistItems[id].textOptionIDs)
		m_text.setColor(device, text, glm::vec3(-1.0f));
	for (int& text : m_picklistItems[id].textAngleBrackets)
		m_text.setColor(device, text, glm::vec3(-1.0f));

	m_quadItems[m_picklistItems[id].quadID].color = glm::vec4(-1.0f);

	addOffsetOnQuads(device, m_picklistItems[id].quadID, -1.0f);
	
	updateQuadUBO(device);

	m_picklistItems[id].activated = false;

	for (int i(0); i < m_picklistItems.size(); ++i)
	{
		if (m_picklistItems[i].masterItemType == MENU_ITEM_TYPE_PICKLIST && m_picklistItems[i].masterItemID == id && m_picklistItems[i].activated)
		{
			hidePicklistItem(device, i);
		}
	}
}

void Menu::hideRangeSliderItem(VkDevice device, int id)
{
	if (!m_rangeSliderItems[id].activated)
		return;

	m_text.setColor(device, m_rangeSliderItems[id].textID, glm::vec3(-1.0f));
	m_quadItems[m_rangeSliderItems[id].quadID].color = glm::vec4(-1.0f);
	m_quadItems[m_rangeSliderItems[id].rangeBarQuadID].color = glm::vec4(-1.0f);
	for (int& sliderBar : m_rangeSliderItems[id].sliderBarQuadIDs)
	{
		m_quadItems[sliderBar].color = glm::vec4(-1.0f);
	}

	addOffsetOnQuads(device, m_rangeSliderItems[id].quadID, -1.0f);

	updateQuadUBO(device);

	m_rangeSliderItems[id].activated = false;
}

void Menu::showBooleanItem(VkDevice device, int id)
{
	m_text.setColor(device, m_booleanItems[id].textID, glm::vec3(1.0f));
	m_text.setColor(device, m_booleanItems[id].textYesID, m_booleanItems[id].value ? TEXT_VALUE_COLOR_YES : TEXT_VALUE_COLOR_NO);
	m_text.setColor(device, m_booleanItems[id].textSeparatorID, TEXT_VALUE_COLOR_NO);
	m_text.setColor(device, m_booleanItems[id].textNoID, m_booleanItems[id].value ? TEXT_VALUE_COLOR_NO : TEXT_VALUE_COLOR_YES);

	m_quadItems[m_booleanItems[id].quadID].color = glm::vec4(ITEM_DEFAULT_COLOR, 1.0f);
	addOffsetOnQuads(device, m_booleanItems[id].quadID, 1.0f);

	updateQuadUBO(device);

	m_booleanItems[id].activated = true;
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
	addOffsetOnQuads(device, m_picklistItems[id].quadID, 1.0f);
	
	updateQuadUBO(device);

	m_picklistItems[id].activated = true;
}

void Menu::showRangeSliderItem(VkDevice device, int id)
{
	m_text.setColor(device, m_rangeSliderItems[id].textID, glm::vec3(1.0f));
	m_quadItems[m_rangeSliderItems[id].quadID].color = glm::vec4(ITEM_DEFAULT_COLOR, 1.0f);
	m_quadItems[m_rangeSliderItems[id].rangeBarQuadID].color = glm::vec4(RANGE_SLIDER_COLOR, 1.0f);
	for (int& sliderBar : m_rangeSliderItems[id].sliderBarQuadIDs)
	{
		m_quadItems[sliderBar].color = glm::vec4(RANGE_SLIDER_ITEM_BAR_COLOR_DEFAULT, 1.0f);
	}

	addOffsetOnQuads(device, m_rangeSliderItems[id].quadID, 1.0f);

	updateQuadUBO(device);

	m_rangeSliderItems[id].activated = true;
}

void Menu::addOffsetOnQuads(VkDevice device, int startID, float offset)
{
	for (int i(startID); i < m_quadItems.size(); ++i)
	{
		if (m_quadItems[i].type == MENU_ITEM_TYPE_UNDEFINED)
			continue;

		const float yTranslation = (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) * offset;
		m_quadItems[i].transform = glm::translate(m_quadItems[i].transform,
			glm::vec3(0.0f, yTranslation / (ITEM_Y_SIZE / 2.0f), 0.0f));
		m_quadItems[i].posOffset.y += yTranslation;

		if (m_quadItems[i].type == MENU_ITEM_TYPE_PICKLIST)
		{
			m_text.translate(device, m_picklistItems[m_quadItems[i].structID].textID, glm::vec2(0.0f, yTranslation));
			for (int& text : m_picklistItems[m_quadItems[i].structID].textOptionIDs)
				m_text.translate(device, text, glm::vec2(0.0f, yTranslation));
			for (int& text : m_picklistItems[m_quadItems[i].structID].textAngleBrackets)
				m_text.translate(device, text, glm::vec2(0.0f, yTranslation));
		}
		else if (m_quadItems[i].type == MENU_ITEM_TYPE_BOOLEAN)
		{
			m_text.translate(device, m_booleanItems[m_quadItems[i].structID].textID, glm::vec2(0.0f, yTranslation));
			m_text.translate(device, m_booleanItems[m_quadItems[i].structID].textNoID, glm::vec2(0.0f, yTranslation));
			m_text.translate(device, m_booleanItems[m_quadItems[i].structID].textSeparatorID, glm::vec2(0.0f, yTranslation));
			m_text.translate(device, m_booleanItems[m_quadItems[i].structID].textYesID, glm::vec2(0.0f, yTranslation));
		}
		else if (m_quadItems[i].type == MENU_ITEM_TYPE_RANGE_SLIDER)
		{
			m_text.translate(device, m_rangeSliderItems[m_quadItems[i].structID].textID, glm::vec2(0.0f, yTranslation));
			m_quadItems[m_rangeSliderItems[m_quadItems[i].structID].rangeBarQuadID].position.y += yTranslation;
			m_quadItems[m_rangeSliderItems[m_quadItems[i].structID].rangeBarQuadID].createTransformMatrix();
			for(int& sliderBar : m_rangeSliderItems[m_quadItems[i].structID].sliderBarQuadIDs)
			{
				m_quadItems[sliderBar].position.y += yTranslation;
				m_quadItems[sliderBar].createTransformMatrix();
			}
			
		}
	}
}

void Menu::updateScrollBar()
{
	// Number of quad drawns
	float nbQuadDrawn = 0.0f;
	for (int i(0); i < m_quadItems.size(); ++i)
		if (m_quadItems[i].color.x >= 0 && m_quadItems[i].type != MENU_ITEM_TYPE_UNDEFINED)
			nbQuadDrawn += 1.0f;

	// Max number of quad drawn on a single scren
	float maxQuadOnScreen = (DRAW_RANGE_Y[1] - DRAW_RANGE_Y[0]) / (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS);

	// Number of screen needed to draw all quads
	float nScreenQuadDrawn = nbQuadDrawn / maxQuadOnScreen;

	float ySizeBar = 1.0f / glm::max(nScreenQuadDrawn, 1.0f);
	float scaleBarY = (DRAW_RANGE_Y[1] - DRAW_RANGE_Y[0]) * ySizeBar;
	m_scroolBarYSize = scaleBarY;
	scaleBarY /= 2.0f; // rectangle is defined in [-1; 1], the scale need to be normalized

	// Translate
	m_scrollBarStartY = DRAW_RANGE_Y[0] + glm::mix(0.0f, (DRAW_RANGE_Y[1] - DRAW_RANGE_Y[0]) / 2.0f, ySizeBar) + m_quadItems[m_scrollBarQuadItemID].posOffset.y;
	m_quadItems[m_scrollBarQuadItemID].transform = glm::translate(glm::mat4(1.0f), glm::vec3(
		/* X */ ITEM_X_OFFSET + ITEM_X_SIZE + DEPENDENT_ITEM_X_OFFSET + SCROLL_BAR_X_OFFSET,
		/* Y */ m_scrollBarStartY,
		/* Z */ 0.0f));
	m_scrollBarStartY -= m_scroolBarYSize / 2.0f;

	// Scale
	m_quadItems[m_scrollBarQuadItemID].transform = glm::scale(m_quadItems[m_quadItems.size() - 1].transform, glm::vec3(SCROLL_BAR_X_SIZE, scaleBarY, 1.0f));

	if (ySizeBar == 1.0f)
		m_quadItems[m_scrollBarQuadItemID].color = glm::vec4(-1.0f);
	else
		m_quadItems[m_scrollBarQuadItemID].color = glm::vec4(m_isScrolling ? SCROOL_BAR_ACTIVE_COLOR :
			m_quadItems[m_scrollBarQuadItemID].onFocus ? SCROOL_BAR_ON_FOCUS_COLOR : SCROOL_BAR_DEFAULT_COLOR, 1.0f);
}

void Menu::setTranformMatrixToAllQuadItems(VkDevice device)
{
	m_scrollTranslation = glm::vec3(m_quadItems[m_scrollBarQuadItemID].posOffset, 0.0f) * (1.0f / (m_scroolBarYSize / 2.0f));

	int quadItemID = -1;
	for (int i(0); i < m_quadItems.size(); ++i)
	{
		if(m_quadItems[i].type == MENU_ITEM_TYPE_UNDEFINED)
			continue;
		quadItemID++;
		
		// Translate
		const float rangeX = ITEM_X_SIZE / 2.0f;
		const float rangeY = ITEM_Y_SIZE / 2.0f;
		m_quadItems[i].transform = glm::translate(glm::mat4(1.0f),
			glm::vec3(ITEM_X_OFFSET + 1.0f - (1.0f - rangeX), ITEM_Y_OFFSET + 1.0f - (1.0f - rangeY) + quadItemID * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS), 0.0f)
			+ glm::vec3(m_quadItems[i].posOffset, 0.0f)
			- m_scrollTranslation);

		// Scale
		m_quadItems[i].transform = glm::scale(m_quadItems[i].transform, glm::vec3(ITEM_X_SIZE / 2.0f, ITEM_Y_SIZE / 2.0f, 1.0f));

		if (m_quadItems[i].type == MENU_ITEM_TYPE_PICKLIST)
		{
			m_text.setPosOffset(device, m_picklistItems[m_quadItems[i].structID].textID, glm::vec2(0.0f, -m_scrollTranslation.y + m_quadItems[i].posOffset.y));
			for (int& text : m_picklistItems[m_quadItems[i].structID].textOptionIDs)
				m_text.setPosOffset(device, text, glm::vec2(0.0f, -m_scrollTranslation.y + m_quadItems[i].posOffset.y));
			for (int& text : m_picklistItems[m_quadItems[i].structID].textAngleBrackets)
				m_text.setPosOffset(device, text, glm::vec2(0.0f, -m_scrollTranslation.y + m_quadItems[i].posOffset.y));
		}
		else if (m_quadItems[i].type == MENU_ITEM_TYPE_BOOLEAN)
		{
			m_text.setPosOffset(device, m_booleanItems[m_quadItems[i].structID].textID, glm::vec2(0.0f, -m_scrollTranslation.y + m_quadItems[i].posOffset.y));
			m_text.setPosOffset(device, m_booleanItems[m_quadItems[i].structID].textNoID, glm::vec2(0.0f, -m_scrollTranslation.y + m_quadItems[i].posOffset.y));
			m_text.setPosOffset(device, m_booleanItems[m_quadItems[i].structID].textSeparatorID, glm::vec2(0.0f, -m_scrollTranslation.y + m_quadItems[i].posOffset.y));
			m_text.setPosOffset(device, m_booleanItems[m_quadItems[i].structID].textYesID, glm::vec2(0.0f, -m_scrollTranslation.y + m_quadItems[i].posOffset.y));
		}
		else if(m_quadItems[i].type == MENU_ITEM_TYPE_RANGE_SLIDER)
		{
			m_text.setPosOffset(device, m_rangeSliderItems[m_quadItems[i].structID].textID, glm::vec2(0.0f, -m_scrollTranslation.y + m_quadItems[i].posOffset.y));
			updateRangeSliderPosition(m_quadItems[i].structID, false);
		}
	}
}
