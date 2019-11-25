#pragma once

#include "VulkanHelper.h"

struct UniformBufferObjectLayout
{
	VkShaderStageFlags accessibility;
	uint32_t binding;
};

template <typename T>
class UniformBufferObject
{
public:
	UniformBufferObject() = default;
	~UniformBufferObject();

	bool initialize(VkDevice device, T data);

private:
};

template<typename T>
inline UniformBufferObject<T>::~UniformBufferObject()
{
}

template<typename T>
inline bool UniformBufferObject<T>::initialize(VkDevice device, T data)
{

	return true;
}
