#include "SceneManager.h"

SceneManager::~SceneManager()
{

}

void SceneManager::load()
{
    for(int i(0); i < 1'000'000; ++i)
    {
        std::cout << "Loading scene\n";
    }

    return;
}
