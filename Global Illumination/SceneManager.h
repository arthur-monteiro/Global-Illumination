#pragma once

#include "LoadingManager.h"

class SceneManager
{
public:
	SceneManager() {}
	~SceneManager();

	bool initialize();

private:
	LoadingManager m_loadingManager;
};

