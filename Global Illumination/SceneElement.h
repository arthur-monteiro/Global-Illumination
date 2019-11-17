#pragma once

#include "VulkanHelper.h"
#include "Model.h"

class SceneElement
{
public:
    SceneElement() = default;
    ~SceneElement();

    bool initialize(VkDevice device);

private:

};