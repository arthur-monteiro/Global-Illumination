#include "Menu.h"

Menu::Menu()
{
}

void Menu::initialize(Vulkan* vk, std::string fontPath)
{
	m_text.initialize(vk, 48, fontPath);

	m_quadFull.loadVertices(vk, VERTEX_QUAD, INDICES_QUAD);

	std::vector<VertexQuadTextured> VERTEX_QUAD_TEXTURED = {
		{ glm::vec2(0.1f, -0.6f), glm::vec2(0.0f, 0.0f) }, // top left
		{ glm::vec2(0.9f, -0.6f), glm::vec2(1.0f, 0.0f) }, // top right
		{ glm::vec2(0.1f, 0.1f), glm::vec2(0.0f, 1.0f) }, // bot left
		{ glm::vec2(0.9f, 0.1f), glm::vec2(1.0f, 1.0f) } // bot right
	};
	m_quadImageOption.loadVertices(vk, VERTEX_QUAD_TEXTURED, INDICES_QUAD);
	m_quadImageOption.createTextureSampler(vk, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

	/*for (int i(0); i < VERTEX_QUAD.size(); ++i)
	{
		VERTEX_QUAD[i].pos /= glm::vec2(2 / ITEM_X_SIZE, 2 / ITEM_Y_SIZE);
		float rangeX = 1.0f / (2 / ITEM_X_SIZE);
		float rangeY = 1.0f / (2 / ITEM_Y_SIZE);
		VERTEX_QUAD[i].pos += glm::vec2(ITEM_X_OFFSET + 1.0f - (1.0f - rangeX), ITEM_Y_OFFSET + 1.0f - (1.0f - rangeY));
	}

	m_quadItem.loadVertices(vk, VERTEX_QUAD, INDICES_QUAD);*/
}

int Menu::addBooleanItem(Vulkan* vk, std::wstring label, std::function<void(void*, bool)> callback, bool defaultValue, void* instance, std::array<std::string, 2> imageOptions)
{
	m_booleanItems.push_back(BooleanItem());
	m_booleanItems[m_booleanItems.size() - 1].value = defaultValue;
	m_booleanItems[m_booleanItems.size() - 1].callback = callback;
	m_booleanItems[m_booleanItems.size() - 1].instance = instance;
	m_booleanItems[m_booleanItems.size() - 1].textID = m_text.addText(vk, label, glm::vec2(ITEM_X_OFFSET + TEXT_X_OFFSET, 
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), TEXT_SIZE,
		glm::vec3(1.0f));
	m_booleanItems[m_booleanItems.size() - 1].quadID = addQuadItem(vk, MENU_ITEM_TYPE_BOOLEAN, m_booleanItems.size() - 1);
	m_booleanItems[m_booleanItems.size() - 1].textNoID = m_text.addText(vk, L"No", glm::vec2(ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(vk, L"Yes  /  No", TEXT_SIZE) / 2.0f,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), TEXT_SIZE,
		defaultValue ? TEXT_VALUE_COLOR_NO : TEXT_VALUE_COLOR_YES);
	m_booleanItems[m_booleanItems.size() - 1].textSeparatorID = m_text.addText(vk, L"/", glm::vec2(
		ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(vk, L"Yes  /  No", TEXT_SIZE) / 2.0f + m_text.simulateSizeX(vk, L"No  ", TEXT_SIZE),
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), TEXT_SIZE,
		TEXT_VALUE_COLOR_NO);
	m_booleanItems[m_booleanItems.size() - 1].textYesID = m_text.addText(vk, L"Yes", glm::vec2(
		ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(vk, L"Yes  /  No", TEXT_SIZE) / 2.0f + m_text.simulateSizeX(vk, L"No  /  ", TEXT_SIZE),
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), TEXT_SIZE,
		defaultValue ? TEXT_VALUE_COLOR_YES : TEXT_VALUE_COLOR_NO);

	m_numberOfItems++;

	for (int i(0); i < 2; ++i)
	{
		if (imageOptions[i] != "")
		{
			m_booleanItems[m_booleanItems.size() - 1].imageOptions[i].loadTextureFromFile(vk, imageOptions[i]);
			m_currentOptionImageView = m_booleanItems[m_booleanItems.size() - 1].imageOptions[i].getImageView();
		}
	}

	return m_booleanItems.size() - 1;
}

int Menu::addPicklistItem(Vulkan* vk, std::wstring label, std::function<void(void*, std::wstring)> callback, void* instance, int defaultValue, std::vector<std::wstring> options)
{
	m_picklistItems.push_back(PicklistItem());
	m_picklistItems[m_picklistItems.size() - 1].selectedOption = defaultValue;
	m_picklistItems[m_picklistItems.size() - 1].options = options;
	m_picklistItems[m_picklistItems.size() - 1].callback = callback;
	m_picklistItems[m_picklistItems.size() - 1].instance = instance;
	m_picklistItems[m_picklistItems.size() - 1].textID = m_text.addText(vk, label, glm::vec2(ITEM_X_OFFSET + TEXT_X_OFFSET,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), TEXT_SIZE,
		glm::vec3(1.0f));
	m_picklistItems[m_picklistItems.size() - 1].quadID = addQuadItem(vk, MENU_ITEM_TYPE_PICKLIST, m_picklistItems.size() - 1);

	m_text.addText(vk, L">", glm::vec2(ITEM_X_OFFSET + ITEM_X_SIZE - 0.035f - m_text.simulateSizeX(vk, L">", TEXT_SIZE) / 2.0f,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), TEXT_SIZE,
		glm::vec3(0.7f));
	m_text.addText(vk, L"<", glm::vec2(ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE + 0.035f - m_text.simulateSizeX(vk, L"<", TEXT_SIZE) / 2.0f,
		ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), TEXT_SIZE,
		glm::vec3(0.7f));

	for (int i(0); i < options.size(); ++i)
	{
		m_picklistItems[m_picklistItems.size() - 1].textOptionIDs.push_back(m_text.addText(vk, options[i], glm::vec2(ITEM_X_OFFSET + ITEM_X_SIZE - ITEM_VALUE_SIZE / 2.0f - m_text.simulateSizeX(vk, options[i], TEXT_SIZE) / 2.0f,
			ITEM_Y_OFFSET + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + (ITEM_Y_SIZE - TEXT_SIZE * 2.0f - 0.012f) / 2.0f), TEXT_SIZE,
			i == defaultValue ? TEXT_VALUE_COLOR_YES : glm::vec3(-1.0f)));
	}

	m_numberOfItems++;
	return m_picklistItems.size() - 1;
}

void Menu::addDependency(int itemTypeSrc, int itemIdSrc, int itemTypeDst, int itemIdDst, std::vector<int> activateValues)
{
	if (itemTypeSrc == MENU_ITEM_TYPE_PICKLIST)
	{
		m_picklistItems[itemIdSrc].activateItemType = itemTypeDst;
		m_picklistItems[itemIdSrc].activateItemID = itemIdDst;
		m_picklistItems[itemIdSrc].valueToActivateItem = activateValues;
	}
}

void Menu::update(Vulkan* vk, int windowWidth, int windowHeight)
{
	double mousePosX, mousePosY;
	glfwGetCursorPos(vk->getWindow(), &mousePosX, &mousePosY);
	// [0, 1]
	mousePosX /= windowWidth; 
	mousePosY /= windowHeight;
	// [0, 2]
	mousePosX *= 2.0;
	mousePosY *= 2.0;
	// [-1, 1]
	mousePosX -= 1.0;
	mousePosY -= 1.0;

	for (int i(0); i < m_quadItems.size(); ++i)
	{
		if (m_quadItems[i].type == MENU_ITEM_TYPE_BOOLEAN && !m_booleanItems[m_quadItems[i].structID].activated)
			continue;

		if (mousePosX > ITEM_X_OFFSET && mousePosX < ITEM_X_OFFSET + ITEM_X_SIZE &&
			mousePosY > ITEM_Y_OFFSET + i * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) && mousePosY < ITEM_Y_OFFSET + i * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS) + ITEM_Y_SIZE)
			setFocus(vk, i, true);
		else
			setFocus(vk, i, false);
	}

	if (glfwGetMouseButton(vk->getWindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && m_oldMouseLeftState == GLFW_RELEASE)
	{
		for (int i(0); i < m_quadItems.size(); ++i)
		{
			if (m_quadItems[i].onFocus)
			{
				switch (m_quadItems[i].type)
				{
				case MENU_ITEM_TYPE_BOOLEAN:
					onClickBooleanItem(vk, m_quadItems[i].structID);
					break;
				case MENU_ITEM_TYPE_PICKLIST:
					onClickPicklistItem(vk, m_quadItems[i].structID);
					break;
				case MENU_ITEM_TYPE_UNDEFINED:
					break;
				default:
					break;
				}
			}
		}
	}

	m_oldMouseLeftState = glfwGetMouseButton(vk->getWindow(), GLFW_MOUSE_BUTTON_1);
}

int Menu::addQuadItem(Vulkan* vk, int type, int id)
{
	m_quadItems.push_back(QuadItem());
	m_quadItems[m_quadItems.size() - 1].type = type;
	m_quadItems[m_quadItems.size() - 1].structID = id;

	m_quadItems[m_quadItems.size() - 1].uboData.color = glm::vec4(ITEM_DEFAULT_COLOR, 1.0f);
	// Translate
	float rangeX = ITEM_X_SIZE / 2.0f;
	float rangeY = ITEM_Y_SIZE / 2.0f;
	m_quadItems[m_quadItems.size() - 1].uboData.transform = glm::translate(glm::mat4(1.0f),
		glm::vec3(ITEM_X_OFFSET + 1.0f - (1.0f - rangeX), ITEM_Y_OFFSET + 1.0f - (1.0f - rangeY) + m_numberOfItems * (ITEM_Y_SIZE + SPACE_BETWEEN_ITEMS), 0.0f));
	// Scale
	m_quadItems[m_quadItems.size() - 1].uboData.transform = glm::scale(m_quadItems[m_quadItems.size() - 1].uboData.transform, glm::vec3(ITEM_X_SIZE / 2.0f, ITEM_Y_SIZE / 2.0f, 1.0f));
	// Load
	m_quadItems[m_quadItems.size() - 1].ubo.load(vk, m_quadItems[m_quadItems.size() - 1].uboData, VK_SHADER_STAGE_VERTEX_BIT);

	return m_quadItems.size() - 1;
}

void Menu::setFocus(Vulkan * vk, int id, bool focus)
{
	if (m_quadItems[id].onFocus != focus)
	{
		m_quadItems[id].onFocus = focus;
		m_quadItems[id].uboData.color = glm::vec4(focus ? ITEM_FOCUS_COLOR : ITEM_DEFAULT_COLOR, 1.0f);
		m_quadItems[id].uboData.transform = m_quadItems[id].uboData.transform = glm::scale(m_quadItems[id].uboData.transform, focus ? glm::vec3(1.05f, 1.05f, 1.0f) : glm::vec3(0.95238f, 0.95238f, 1.0f));
		m_quadItems[id].ubo.update(vk, m_quadItems[id].uboData);
	}
}

void Menu::onClickBooleanItem(Vulkan* vk, int id)
{
	m_booleanItems[id].value = m_booleanItems[id].value ? false : true;
	m_quadItems[m_booleanItems[id].quadID].uboData.color = glm::vec4(m_booleanItems[id].value ? TEXT_VALUE_COLOR_YES : TEXT_VALUE_COLOR_NO, 1.0f);

	m_text.setColor(vk, m_booleanItems[id].textYesID, m_booleanItems[id].value ? TEXT_VALUE_COLOR_YES : TEXT_VALUE_COLOR_NO);
	m_text.setColor(vk, m_booleanItems[id].textNoID, m_booleanItems[id].value ? TEXT_VALUE_COLOR_NO : TEXT_VALUE_COLOR_YES);

	m_currentOptionImageView = m_booleanItems[id].imageOptions[m_booleanItems[id].value ? 1 : 0].getImageView();

	m_booleanItems[id].callback(m_booleanItems[id].instance, m_booleanItems[id].value);
}

void Menu::onClickPicklistItem(Vulkan* vk, int id)
{
	m_text.setColor(vk, m_picklistItems[id].textOptionIDs[m_picklistItems[id].selectedOption], glm::vec3(-1.0f));
	m_picklistItems[id].selectedOption = (m_picklistItems[id].selectedOption + 1) % m_picklistItems[id].options.size();
	m_text.setColor(vk, m_picklistItems[id].textOptionIDs[m_picklistItems[id].selectedOption], TEXT_VALUE_COLOR_YES);

	bool needActivate = false;
	for(int i(0); i < m_picklistItems[id].valueToActivateItem.size(); ++i)
		if (m_picklistItems[id].valueToActivateItem[i] == m_picklistItems[id].selectedOption)
		{
			needActivate = true;
			break;
		}

	if (m_picklistItems[id].activateItemType == MENU_ITEM_TYPE_BOOLEAN && 
		((needActivate && !m_booleanItems[m_picklistItems[id].activateItemID].activated) || (!needActivate && m_booleanItems[m_picklistItems[id].activateItemID].activated)))
	{
		if (needActivate)
		{
			m_quadItems[m_booleanItems[m_picklistItems[id].activateItemID].quadID].uboData.color = glm::vec4(ITEM_DEFAULT_COLOR, 1.0f);
			m_quadItems[m_booleanItems[m_picklistItems[id].activateItemID].quadID].ubo.update(vk, m_quadItems[m_booleanItems[m_picklistItems[id].activateItemID].quadID].uboData);

			m_text.setColor(vk, m_booleanItems[m_picklistItems[id].activateItemID].textID, TEXT_VALUE_COLOR_YES);
			m_text.setColor(vk, m_booleanItems[m_picklistItems[id].activateItemID].textNoID, m_booleanItems[m_picklistItems[id].activateItemID].value ? TEXT_VALUE_COLOR_NO : TEXT_VALUE_COLOR_YES);
			m_text.setColor(vk, m_booleanItems[m_picklistItems[id].activateItemID].textSeparatorID, TEXT_VALUE_COLOR_NO);
			m_text.setColor(vk, m_booleanItems[m_picklistItems[id].activateItemID].textYesID, m_booleanItems[m_picklistItems[id].activateItemID].value ? TEXT_VALUE_COLOR_YES : TEXT_VALUE_COLOR_NO);

			m_booleanItems[m_picklistItems[id].activateItemID].activated = true;
		}
		else
		{
			m_quadItems[m_booleanItems[m_picklistItems[id].activateItemID].quadID].uboData.color = glm::vec4(ITEM_DEACTIVATED_COLOR, 1.0f);
			m_quadItems[m_booleanItems[m_picklistItems[id].activateItemID].quadID].ubo.update(vk, m_quadItems[m_booleanItems[m_picklistItems[id].activateItemID].quadID].uboData);

			m_text.setColor(vk, m_booleanItems[m_picklistItems[id].activateItemID].textID, TEXT_COLOR_DEACTIVATED);
			m_text.setColor(vk, m_booleanItems[m_picklistItems[id].activateItemID].textNoID, TEXT_COLOR_DEACTIVATED);
			m_text.setColor(vk, m_booleanItems[m_picklistItems[id].activateItemID].textSeparatorID, TEXT_COLOR_DEACTIVATED);
			m_text.setColor(vk, m_booleanItems[m_picklistItems[id].activateItemID].textYesID, TEXT_COLOR_DEACTIVATED);

			m_booleanItems[m_picklistItems[id].activateItemID].activated = false;
		}
	}

	m_currentOptionImageView = VK_NULL_HANDLE;

	m_picklistItems[id].callback(m_picklistItems[id].instance, m_picklistItems[id].options[m_picklistItems[id].selectedOption]);
}
