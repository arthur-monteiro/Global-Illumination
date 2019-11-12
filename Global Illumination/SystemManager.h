/*
  Manager for Scene, GLFW and Vulkan
*/

#pragma once

#include "SceneManager.h"
#include "WindowManager.h"
#include "Vulkan.h"
#include "SwapChain.h"

class SystemManager
{
public:
	SystemManager() {}
	~SystemManager();

	bool initialize();
	bool run();
	bool cleanup();

private:
	static void windowResizeCallback(void* systemManagerInstance, int width, int height)
	{

	}

private:
	SceneManager m_sceneManager;
	WindowManager m_windowManager;
	Vulkan m_vulkan;
	SwapChain m_swapChain;
};
	