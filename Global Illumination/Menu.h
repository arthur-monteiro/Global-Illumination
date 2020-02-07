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

class Menu
{
public:
	Menu() = default;
	~Menu() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D outputExtent,
		std::function<void(void*, VkImageView)> callbackSetImageView, void* instance);
	int addBooleanItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, bool)> callback, 
		bool defaultValue, void* instance, std::array<std::string, 2> imageOptions, Font* font);
	int addPicklistItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, std::wstring)> callback, 
		void* instance, int defaultValue, std::vector<std::wstring> options, Font* font, glm::vec2 offset = glm::vec2(0.0f));
	int addDependentPicklistItem(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::wstring label, std::function<void(void*, std::wstring)> callback,
	                             void* instance, int defaultValue, std::vector<std::wstring> options, Font* font, int itemTypeSrc, int itemIdSrc, std::vector<int> activateValues);
	/*void addDependency(int itemTypeSrc, int itemIdSrc, int itemTypeDst, int itemIdDst, std::vector<int> activateValues);*/

	void build(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, Font* font);

	void update(VkDevice device, GLFWwindow* window, int windowWidth, int windowHeight);

	void cleanup(VkDevice device, VkDescriptorPool descriptorPool);

	std::vector<Renderer*> getRenderers() { return { &m_quadRenderer, &m_textRenderer}; }

private:	
	int addQuadItem(int type, int id, glm::vec2 offset = glm::vec2(0.0f));
	bool setFocus(int id, bool focus); // Returns if a change has been done
	void updateQuadUBO(VkDevice device);
	
	void onClickBooleanItem(VkDevice device, int id);
	void onClickPicklistItem(VkDevice device, int id);

	void hidePicklistItem(VkDevice device, int id);
	void showPicklistItem(VkDevice device, int id);

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
	const float DEPENDENT_ITEM_X_OFFSET = 0.1f;

	const float TEXT_X_OFFSET = 0.03f;
	const float TEXT_SIZE = 0.028f;

	const glm::vec3 TEXT_VALUE_COLOR_NO = glm::vec3(0.2f);
	const glm::vec3 TEXT_VALUE_COLOR_YES = glm::vec3(1.0f);
	const glm::vec3 TEXT_COLOR_DEACTIVATED = glm::vec3(0.3f);

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

	struct QuadItem
	{
		glm::vec4 color;
		glm::mat4 transform;
		glm::vec2 posOffset = glm::vec2(0.0f);

		bool onFocus = false;

		int type = MENU_ITEM_TYPE_UNDEFINED;
		int structID = -1; // id of the "logic" data
	};

	Text m_text;
	//Font m_font;
	Renderer m_textRenderer;
	
	Mesh<Vertex2D> m_quad;
	Mesh<Vertex2DTextured> m_quadImageOption;
	Instance<InstanceSingleID> m_quadInstances;

	struct UniformBufferObjectQuads
	{
		glm::mat4 transform[32];
		glm::vec4 color[32];
	};
	UniformBufferObjectQuads m_uboQuadsData;
	UniformBufferObject m_uboQuads;
	Renderer m_quadRenderer;

	int m_numberOfItems = 0;
	std::vector<BooleanItem> m_booleanItems;
	std::vector<PicklistItem> m_picklistItems;

	std::vector<QuadItem> m_quadItems;

	int m_oldMouseLeftState = GLFW_RELEASE;
	VkImageView m_currentOptionImageView = VK_NULL_HANDLE;

	bool m_oldSomeoneOnFocus = false;
	std::function<void(void*, VkImageView)> m_callbackSetImageView;
	void* m_callerInstance;
	VkExtent2D m_outputExtent;
};