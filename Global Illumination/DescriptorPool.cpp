#include "DescriptorPool.h"

DescriptorPool::~DescriptorPool()
{
}

void DescriptorPool::initialize(VkDevice device)
{
	std::array<VkDescriptorPoolSize, 6> poolSizes;
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 512;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 512;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[2].descriptorCount = 1024;
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	poolSizes[3].descriptorCount = 16;
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	poolSizes[4].descriptorCount = 1024;
	poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[5].descriptorCount = 16;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1024;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Error : create descriptor pool");
}

void DescriptorPool::cleanup(VkDevice device)
{
	vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
}
