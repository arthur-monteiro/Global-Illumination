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

const int SCENE_TYPE_UNDEFINED = -1;
const int SCENE_TYPE_SHADOWMAP = 0;
const int SCENE_TYPE_NO_SHADOW = 1;

class System
{
public:
	System();
	~System();

	void initialize();
	bool mainLoop();
	void cleanup();

	void changePCF(bool status);
	void drawFPSCounter(bool status);
	void changeShadows(std::wstring value);

	static void recreateCallback(void* instance) { reinterpret_cast<System*>(instance)->create(true); }
	static void changePCFCallback(void* instance, bool status) { reinterpret_cast<System*>(instance)->changePCF(status); }
	static void drawFPSCounterCallback(void* instance, bool status) { reinterpret_cast<System*>(instance)->drawFPSCounter(status); }
	static void changeShadowsCallback(void* instance, std::wstring value) { reinterpret_cast<System*>(instance)->changeShadows(value); }
private:
	void create(bool recreate = false);
	void createRessources();
	void createPasses(int type, bool recreate = false);

private:
	Vulkan m_vk;

	RenderPass m_swapChainRenderPass;
	RenderPass m_offscreenShadowMap;
	int m_sceneType = SCENE_TYPE_UNDEFINED;

	MeshPBR m_skybox;
	MeshPBR m_wall;
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

	UniformBufferObject<UniformBufferObjectVP> m_uboVP;
	UniformBufferObjectVP m_uboVPData;
	UniformBufferObject<UniformBufferObjectVP> m_uboVPShadowMap;
	UniformBufferObject<UniformBufferObjectSingleMat> m_uboLightSpace;

	UniformBufferObject<UniformBufferObjectDirLight> m_uboDirLight;
	UniformBufferObjectDirLight m_uboDirLightData;

	UniformBufferObject<UniformBufferObjectVP> m_uboVPSkybox;
	UniformBufferObjectVP m_uboVPSkyboxData;

	Camera m_camera;
	int m_oldEscapeState = GLFW_RELEASE;
};

