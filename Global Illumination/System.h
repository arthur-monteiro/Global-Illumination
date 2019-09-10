#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <chrono>
#include <memory>
#include <algorithm>

#include "Vulkan.h"
#include "RenderPass.h"
#include "Mesh.h"
#include "Text.h"
#include "UniformBufferObject.h"
#include "Camera.h"
#include "Instance.h"
#include "Model.h"

const int SCENE_TYPE_UNDEFINED = -1;
const int SCENE_TYPE_SHADOWMAP = 0;
const int SCENE_TYPE_NO_SHADOW = 1;
const int SCENE_TYPE_CASCADED_SHADOWMAP = 2;

class System
{
public:
	System();
	~System();

	void initialize();
	bool mainLoop();
	void cleanup();

	void setMenuOptionImageView(VkImageView imageView);
	void changePCF(bool status);
	void drawFPSCounter(bool status);
	void changeShadows(std::wstring value);
	void changeGlobalIllumination(std::wstring value);
	void changeMSAA(std::wstring value);

	static void recreateCallback(void* instance) { reinterpret_cast<System*>(instance)->create(true); }
	static void setMenuOptionImageViewCallback(void* instance, VkImageView imageView) { reinterpret_cast<System*>(instance)->setMenuOptionImageView(imageView); }
	static void changePCFCallback(void* instance, bool status) { reinterpret_cast<System*>(instance)->changePCF(status); }
	static void drawFPSCounterCallback(void* instance, bool status) { reinterpret_cast<System*>(instance)->drawFPSCounter(status); }
	static void changeShadowsCallback(void* instance, std::wstring value) { reinterpret_cast<System*>(instance)->changeShadows(value); }
	static void changeMSAACallback(void* instance, std::wstring value) { reinterpret_cast<System*>(instance)->changeMSAA(value); }
	static void changeGlobalIlluminationCallback(void* instance, std::wstring value) { reinterpret_cast<System*>(instance)->changeGlobalIllumination(value); }
private:
	void create(bool recreate = false);
	void createRessources();
	void createPasses(int type, VkSampleCountFlagBits msaaSamples, bool recreate = false);

	void updateCSM();

private:
	Vulkan m_vk;

	glm::vec3 m_lightDir = glm::vec3(-1.0f, -1.0f, 0.0f);
	int m_cascadeCount = 3;
	std::vector<float> m_cascadeSplits;

	RenderPass m_swapChainRenderPass;
	RenderPass m_offscreenShadowMap;
	RenderPass m_offscreenCascadedShadowMap;
	int m_sceneType = SCENE_TYPE_UNDEFINED;

	MeshPBR m_skybox;
	MeshPBR m_wall;
	Model m_sponza;
	MeshPBR m_quad;
	Text m_text;
	Menu m_menu;

	int m_fpsCounterTextID = -1;
	int m_fpsCount = 0;
	std::chrono::steady_clock::time_point m_startTimeFPSCounter = std::chrono::steady_clock::now();

	double m_oldMousePosX = 0;
	double m_oldMousePosY = 0;
	bool m_wasClickPressed = 0;

	int m_skyboxID;

	/* Final pass*/
	UniformBufferObject<UniformBufferObjectVP> m_uboVP;
	UniformBufferObjectVP m_uboVPData;
	UniformBufferObject<UniformBufferObjectSingleMat> m_uboModel;
	UniformBufferObjectSingleMat m_uboModelData;

	/* Light */
	UniformBufferObject<UniformBufferObjectDirLight> m_uboDirLight;
	UniformBufferObjectDirLight m_uboDirLightData;

	UniformBufferObject<UniformBufferObjectDirLightCSM> m_uboDirLightCSM;
	UniformBufferObjectDirLightCSM m_uboDirLightCSMData;

	/* Shadow Map */
	UniformBufferObject<UniformBufferObjectVP> m_uboVPShadowMap;
	UniformBufferObject<UniformBufferObjectSingleMat> m_uboLightSpace;

	/* CSM */
	std::vector<UniformBufferObject<UniformBufferObjectVP>> m_uboVPCSM;
	std::vector<UniformBufferObjectVP> m_uboVPCSMData;
	UniformBufferObject<UniformBufferObjectArrayMat> m_uboLightSpaceCSM;
	UniformBufferObjectArrayMat m_uboLightSpaceCSMData;

	/* Unused*/
	UniformBufferObject<UniformBufferObjectVP> m_uboVPSkybox;
	UniformBufferObjectVP m_uboVPSkyboxData;

	glm::vec3 m_sunDirection = glm::vec3(0.0f, -1.0f, 0.0f);

	Camera m_camera;
	int m_oldEscapeState = GLFW_RELEASE;
	VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

