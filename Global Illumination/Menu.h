#pragma once

#include "Pipeline.h"
#include "Text.h"
#include "Mesh.h"
#include "UniformBufferObject.h"
#include "Texture.h"
#include "Renderer.h"
#include "Instance.h"

#include <array>

const int MENU_ITEM_TYPE_UNDEFINED = -1;
const int MENU_ITEM_TYPE_BOOLEAN = 0;
const int MENU_ITEM_TYPE_PICKLIST = 1;
const int MENU_ITEM_TYPE_RANGE_SLIDER = 2;

class Menu
{
public:
	Menu() = default;
	~Menu() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D outputExtent,
		std::function<void(void*, VkImageView)> callbackSetImageView, void* instance);
	int addBooleanItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, bool)> callback,
		bool defaultValue, void* instance, std::array<std::string, 2> imageOptions, Font* font, glm::vec2 offset = glm::vec2(0.0f));
	int addDependentBooleanItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, bool)> callback,
		bool defaultValue, void* instance, std::array<std::string, 2> imageOptions, Font* font, int itemTypeSrc, int itemIdSrc, std::vector<int> activateValues);
	int addPicklistItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, std::wstring)> callback,
		void* instance, int defaultValue, std::vector<std::wstring> options, Font* font, glm::vec2 offset = glm::vec2(0.0f));
	int addDependentPicklistItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, std::wstring)> callback,
		void* instance, int defaultValue, std::vector<std::wstring> options, Font* font, int itemTypeSrc, int itemIdSrc, std::vector<int> activateValues);
	int addRangeSliderItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, std::vector<float>)> callback,
		void* instance, std::vector<float> defaultValues, float rangeStart, float rangeEnd, Font* font, glm::vec2 offset = glm::vec2(0.0f));
	int addDependentRangeSliderItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, std::vector<float>)> callback,
		void* instance, std::vector<float> defaultValues, float rangeStart, float rangeEnd, Font* font, int itemTypeSrc, int itemIdSrc, std::vector<int> activateValues);
	/*void addDependency(int itemTypeSrc, int itemIdSrc, int itemTypeDst, int itemIdDst, std::vector<int> activateValues);*/

	void disableItem(VkDevice device, int itemType, int itemId);
	void enableItem(VkDevice device, int itemType, int itemId);

	void build(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, Font* font);

	void update(VkDevice device, GLFWwindow* window, int windowWidth, int windowHeight);

	void cleanup(VkDevice device, VkDescriptorPool descriptorPool);

	std::vector<Renderer*> getRenderers() { return { &m_quadRenderer, &m_textRenderer }; }

private:
	int addQuadItem(int type, int id, glm::vec2 offset = glm::vec2(0.0f));
	bool setFocus(int id, bool focus); // Returns if a change has been done
	void updateQuadUBO(VkDevice device);

	void onClickBooleanItem(VkDevice device, int id);
	void onClickPicklistItem(VkDevice device, int id);
	void onClickRangeSliderItem(VkDevice device, int id, double posX, double posY);

	void hideBooleanItem(VkDevice device, int id);
	void hidePicklistItem(VkDevice device, int id);
	void hideRangeSliderItem(VkDevice device, int id);
	void showBooleanItem(VkDevice device, int id);
	void showPicklistItem(VkDevice device, int id);
	void showRangeSliderItem(VkDevice device, int id);

	void addOffsetOnQuads(VkDevice device, int startID, float offset);

	void updateScrollBar();
	void setTranformMatrixToAllQuadItems(VkDevice device);

	void updateRangeSliderPosition(int id, bool changeColor);

private:
	const float DRAW_RANGE_Y[2] = { -0.7f, 0.8f };

	const float ITEM_X_OFFSET = -0.83f;
	const float ITEM_Y_OFFSET = -0.7f;
	const float ITEM_X_SIZE = 0.85f;
	const float ITEM_Y_SIZE = 0.15f;
	const glm::vec3 ITEM_DEFAULT_COLOR = glm::vec3(0.3f, 0.3f, 0.3f);
	const glm::vec3 ITEM_FOCUS_COLOR = glm::vec3(1.0f, 0.3f, 0.3f);
	const glm::vec3 ITEM_DEACTIVATED_COLOR = glm::vec3(0.2f, 0.2f, 0.2f);
	const float SPACE_BETWEEN_ITEMS = 0.05f;
	const float ITEM_VALUE_SIZE = 0.38f;
	const float DEPENDENT_ITEM_X_OFFSET = 0.1f;

	const float TEXT_X_OFFSET = 0.03f;
	const float TEXT_SIZE = 0.028f;

	const glm::vec3 TEXT_VALUE_COLOR_NO = glm::vec3(0.2f);
	const glm::vec3 TEXT_VALUE_COLOR_YES = glm::vec3(1.0f);
	const glm::vec3 TEXT_COLOR_DEACTIVATED = glm::vec3(0.3f);

	const float SCROLL_BAR_X_OFFSET = 0.07f;
	const float SCROLL_BAR_X_SIZE = 0.01f;
	const glm::vec3 SCROOL_BAR_DEFAULT_COLOR = glm::vec3(0.3f);
	const glm::vec3 SCROOL_BAR_ON_FOCUS_COLOR = glm::vec3(0.35f);
	const glm::vec3 SCROOL_BAR_ACTIVE_COLOR = glm::vec3(0.4f);

	const float RANGE_SLIDER_OFFSET_LEFT = 0.4f;
	const float RANGE_SLIDER_OFFSET_RIGHT = 0.05f;
	const float RANGE_SLIDER_Y_SIZE = 0.015f;
	const glm::vec3 RANGE_SLIDER_COLOR = glm::vec3(0.5f);
	const float RANGE_SLIDER_ITEM_BAR_X_SIZE = 0.01f;
	const float RANGE_SLIDER_ITEM_BAR_Y_SIZE = 0.05f;
	const glm::vec3 RANGE_SLIDER_ITEM_BAR_COLOR_DEFAULT = glm::vec3(0.7f);
	const glm::vec3 RANGE_SLIDER_ITEM_BAR_COLOR_ON_FOCUS = glm::vec3(0.75f);
	const glm::vec3 RANGE_SLIDER_ITEM_BAR_COLOR_ON_CLICK = glm::vec3(0.8f);

	std::vector<Vertex2D> VERTEX_QUAD = {
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

		int masterItemType = MENU_ITEM_TYPE_UNDEFINED;
		int masterItemID = -1;
		std::vector<int> masterValuesActivation;

		bool activated = true;

		std::array<Texture, 2> texturesOptions;
	};

	struct PicklistItem
	{
		std::vector<std::wstring> options;
		int selectedOption;
		std::function<void(void*, std::wstring)> callback;
		void* instance = nullptr;
		int textID = -1;
		int quadID = -1;

		std::array<int, 2> textAngleBrackets;
		std::vector<int> textOptionIDs;

		//int activateItemType = MENU_ITEM_TYPE_UNDEFINED;
		//int activateItemID = -1;

		int masterItemType = MENU_ITEM_TYPE_UNDEFINED;
		int masterItemID = -1;
		std::vector<int> masterValuesActivation;

		std::vector<Texture> texturesOptions;

		bool activated = true;
	};

	struct RangeSliderItem
	{
		float start;
		float end;
		std::vector<float> values;
		std::function<void(void*, std::vector<float>)> callback;
		void* instance = nullptr;
		int textID = -1;
		int quadID = -1;

		int rangeBarQuadID = -1;
		std::vector<int> sliderBarQuadIDs;

		int masterItemType = MENU_ITEM_TYPE_UNDEFINED;
		int masterItemID = -1;
		std::vector<int> masterValuesActivation;

		bool activated = true;
	};

	struct QuadItem
	{
		glm::vec4 color;
		glm::mat4 transform;
		glm::vec2 posOffset = glm::vec2(0.0f);

		bool onFocus = false;

		int type = MENU_ITEM_TYPE_UNDEFINED;
		int structID = -1; // id of the "logic" data

		glm::vec2 position;
		glm::vec2 size;

		void createTransformMatrix()
		{
			transform = glm::translate(glm::mat4(1.0f),	glm::vec3(position.x, position.y, 0.0f));
			transform = glm::scale(transform, glm::vec3(size.x / 2.0f, size.y / 2.0f, 1.0f));
		}
	};

	Text m_text;
	//Font m_font;
	Renderer m_textRenderer;

	Mesh<Vertex2D> m_quad;
	Mesh<Vertex2DTextured> m_quadImageOption;
	Instance<InstanceSingleID> m_quadInstances;

	struct UniformBufferObjectQuads
	{
		glm::mat4 transform[64];
		glm::vec4 color[64];
	};
	UniformBufferObjectQuads m_uboQuadsData;
	UniformBufferObject m_uboQuads;
	Renderer m_quadRenderer;

	int m_numberOfItems = 0;
	std::vector<BooleanItem> m_booleanItems;
	std::vector<PicklistItem> m_picklistItems;

	// Range sliders
	std::vector<RangeSliderItem> m_rangeSliderItems;
	int m_rangeSliderMovingID = -1;
	int m_rangeSliderMovingSlideBarNum = -1;

	std::vector<QuadItem> m_quadItems;

	int m_oldMouseLeftState = GLFW_RELEASE;
	VkImageView m_currentOptionImageView = VK_NULL_HANDLE;

	bool m_oldSomeoneOnFocus = false;
	std::function<void(void*, VkImageView)> m_callbackSetImageView;
	void* m_callerInstance;
	VkExtent2D m_outputExtent;

	// Scroll bar
	int m_scrollBarQuadItemID = -1;
	float m_scrollBarStartY = 0.0f;
	float m_scroolBarYSize = 0.0f;
	bool m_isScrolling = false;
	glm::vec2 m_previousMousePosForScrolling = glm::vec2(0.0f);
	glm::vec3 m_scrollTranslation = glm::vec3(0.0f);
};