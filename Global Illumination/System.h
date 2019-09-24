#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <chrono>
#include <memory>
#include <algorithm>
#include <limits>
#include <iostream>

#include "Vulkan.h"
#include "RenderPass.h"
#include "ComputePass.h"
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
	void createUniformBufferObjects();
	void setSemaphores();

	void updateCSM();

private:
	Vulkan m_vk;

	glm::vec3 m_lightDir = glm::vec3(0.0f, -5.0f, -1.0f);

	/* ----- Render Pass ------ */
    int m_sceneType = SCENE_TYPE_UNDEFINED;

	/* Final Pass */
	RenderPass m_swapChainRenderPass;

	/* Shadow Mapping */
	RenderPass m_offscreenShadowMap;

	/* CSM */
	RenderPass m_offscreenCascadedShadowMap;
	//std::vector<std::pair<VkBuffer, VkDeviceMemory>> m_shadowMapBuffers;
	//std::vector<Image> m_shadowMapImages;
	//std::vector<Image> m_shadowMapImagesDowngraded;
	Image m_shadowScreenImage;
	RenderPass m_offscreenShadowCalculation;
	ComputePass m_offscreenShadowBlur;
    int m_cascadeCount = 4;
    std::vector<float> m_cascadeSplits;

	/* ----- Meshes / Models ----- */

	/* Final Pass */
	MeshPBR m_wall;
	Model m_sponza;
	Text m_text;
	Menu m_menu;

	/* Shadow Mapping */
	MeshPBR m_quad;

	/* CSM */
	Mesh2DTextured m_csmBlurSquare;

	/* Unused */
	MeshPBR m_skybox;


	int m_fpsCounterTextID = -1;
	int m_fpsCount = 0;
	std::chrono::steady_clock::time_point m_startTimeFPSCounter = std::chrono::steady_clock::now();

	double m_oldMousePosX = 0;
	double m_oldMousePosY = 0;
	bool m_wasClickPressed = 0;

	int m_skyboxID;


	/* ----- Uniform Buffer Objects ----- */

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
	UniformBufferObject<UniformBufferObjectCSM> m_uboCascadeSplits;
	UniformBufferObjectCSM m_uboCascadeSplitsData;

	/* Unused*/
	UniformBufferObject<UniformBufferObjectVP> m_uboVPSkybox;
	UniformBufferObjectVP m_uboVPSkyboxData;

	Camera m_camera;
	int m_oldEscapeState = GLFW_RELEASE;
	VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

