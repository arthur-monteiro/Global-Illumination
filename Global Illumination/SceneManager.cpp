#include "SceneManager.h"

SceneManager::~SceneManager()
{
}

bool SceneManager::initialize()
{
	m_loadingManager.initialize();

	return true;
}
