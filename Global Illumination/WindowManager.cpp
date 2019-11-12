#include "WindowManager.h"

WindowManager::~WindowManager()
{

}

bool WindowManager::initialize(std::string appName, int width, int height, void* systemManagerInstance,
	std::function<void(void*, int, int)> resizeCallback)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_window = glfwCreateWindow(width, height, appName.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(m_window, this);
	glfwSetWindowSizeCallback(m_window, WindowManager::onWindowResized);
	//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	m_systemManagerInstance = systemManagerInstance;
	m_resizeCallback = resizeCallback;

	return true;
}
