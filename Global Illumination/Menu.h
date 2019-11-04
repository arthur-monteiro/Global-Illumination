#pragma once

#include "Pipeline.h"
#include "Text.h"
#include "Mesh.h"
#include "UniformBufferObject.h"
#include "Image.h"

const int MENU_ITEM_TYPE_UNDEFINED = -1;
const int MENU_ITEM_TYPE_BOOLEAN = 0;
const int MENU_ITEM_TYPE_PICKLIST = 1;

class Menu
{
public:
	Menu();

	void initialize(Vulkan* vk, std::string fontPath, std::function<void(void*, VkImageView)> callbackSetImageView, void* instance);
	int addBooleanItem(Vulkan* vk, std::wstring label, std::function<void(void*, bool)> callback, bool defaultValue, void* instance, std::array<std::string, 2> imageOptions);
	int addPicklistItem(Vulkan* vk, std::wstring label, std::function<void(void*, std::wstring)> callback, void* instance, int defaultValue, 
		std::vector<std::wstring> options);
	void addDependency(int itemTypeSrc, int itemIdSrc, int itemTypeDst, int itemIdDst, std::vector<int> activateValues);

	void update(Vulkan* vk, int windowWidth, int windowHeight);

	void cleanup(VkDevice device);

	Text getText() { return m_text; }
	Mesh2D* getQuadFull() { return &m_quadFull; }
	std::vector<UboBase*> getUBOs()
	{
		std::vector<UboBase*> r;
		for (int i(0); i < m_quadItems.size(); ++i)
			r.push_back(&m_quadItems[i].ubo);

		return r;
	}
	Mesh2DTextured* getQuadImageOption() { return &m_quadImageOption; }
	VkImageView getOptionImageView() { return m_currentOptionImageView; }

private:
	int addQuadItem(Vulkan* vk, int type, int id);
	void setFocus(Vulkan* vk, int id, bool focus);

	void onClickBooleanItem(Vulkan* vk, int id);
	void onClickPicklistItem(Vulkan* vk, int id);

private:
	const float ITEM_X_OFFSET = -0.83f;
	const float ITEM_Y_OFFSET = -0.7f;
	const float ITEM_X_SIZE = 0.85f;
	const float ITEM_Y_SIZE = 0.15f;
	const glm::vec3 ITEM_DEFAULT_COLOR = glm::vec3(0.3f, 0.3f, 0.3f);
	const glm::vec3 ITEM_FOCUS_COLOR = glm::vec3(1.0f, 0.3f, 0.3f);
	const glm::vec3 ITEM_DEACTIVATED_COLOR = glm::vec3(0.2f, 0.2f, 0.2f);
	const float SPACE_BETWEEN_ITEMS = 0.05f;
	const float ITEM_VALUE_SIZE = 0.38f;

	const float TEXT_X_OFFSET = 0.03f;
	const float TEXT_SIZE = 0.028f;

	const glm::vec3 TEXT_VALUE_COLOR_NO = glm::vec3(0.1f);
	const glm::vec3 TEXT_VALUE_COLOR_YES = glm::vec3(1.0f);
	const glm::vec3 TEXT_COLOR_DEACTIVATED = glm::vec3(0.3f);

	std::vector<VertexQuad> VERTEX_QUAD = {
		{ glm::vec2(-1.0f, -1.0f) }, // bot left
		{ glm::vec2(-1.0f, 1.0f) }, // top left
		{ glm::vec2(1.0f, -1.0f) }, // bot right
		{ glm::vec2(1.0f, 1.0f) } // top right
	};
	const std::vector<uint32_t> INDICES_QUAD = {
			0, 1, 2,
			1, 3, 2
	};

	struct BooleanItem
	{
		bool value;
		std::function<void(void*, bool)> callback;
		void* instance = nullptr;
		int textID = -1;
		int quadID = -1;

		int textYesID = -1;
		int textSeparatorID = -1;
		int textNoID = -1;

		bool activated = true;

		std::array<Image, 2> imageOptions;
	};

	struct PicklistItem
	{
		std::vector<std::wstring> options;
		int selectedOption;
		std::function<void(void*, std::wstring)> callback;
		void* instance = nullptr;
		int textID = -1;
		int quadID = -1;

		std::vector<int> textOptionIDs;

		int activateItemType = MENU_ITEM_TYPE_UNDEFINED;
		int activateItemID = -1;
		std::vector<int> valueToActivateItem;

		std::vector<Image> imageOptions;
	};

	struct QuadItem
	{
		UniformBufferObject<UniformBufferObjectItemQuad> ubo;
		UniformBufferObjectItemQuad uboData;

		bool onFocus = false;

		int type = MENU_ITEM_TYPE_UNDEFINED;
		int structID = -1;
	};

	Text m_text;
	Mesh2D m_quadFull;
	Mesh2DTextured m_quadImageOption;

	int m_numberOfItems = 0;
	std::vector<BooleanItem> m_booleanItems;
	std::vector<PicklistItem> m_picklistItems;

	std::vector<QuadItem> m_quadItems;

	int m_oldMouseLeftState = GLFW_RELEASE;
	VkImageView m_currentOptionImageView = VK_NULL_HANDLE;

	bool m_oldSomeoneOnFocus = false;
	std::function<void(void*, VkImageView)> m_callbackSetImageView;
	void* m_callerInstance;
};

