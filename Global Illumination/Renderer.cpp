#include "Renderer.h"

Renderer::~Renderer()
{
}

void Renderer::initialize(VkDevice device, std::string vertexShader, std::string fragmentShader, std::vector<VkVertexInputBindingDescription> vertexInputDescription,
	std::vector<VkVertexInputAttributeDescription> attributeInputDescription, std::vector<UniformBufferObjectLayout> uniformBufferObjectLayouts, 
	std::vector<TextureLayout> textureLayouts, std::vector<bool> alphaBlending)
{
	m_vertexShader = vertexShader;
	m_fragmentShader = fragmentShader;
	m_vertexInputDescription = vertexInputDescription;
	m_attributeInputDescription = attributeInputDescription;
	m_alphaBlending = alphaBlending;

	createDescriptorSetLayout(device, uniformBufferObjectLayouts, textureLayouts);
}

void Renderer::createPipeline(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, VkSampleCountFlagBits msaa)
{
	if (m_pipelineCreated)
		return;

	m_pipeline.initialize(device, renderPass, m_vertexShader, m_fragmentShader, m_vertexInputDescription, m_attributeInputDescription, extent, msaa, m_alphaBlending, &m_descriptorSetLayout);
	m_pipelineCreated = true;
}

void Renderer::destroyPipeline(VkDevice device)
{
	if (!m_pipelineCreated)
		return;

	m_pipeline.cleanup(device);
	m_pipelineCreated = false;
}

int Renderer::addMesh(VkDevice device, VkDescriptorPool descriptorPool, VertexBuffer vertexBuffer,
	std::vector<std::pair<UniformBufferObject*, UniformBufferObjectLayout>> ubos, std::vector<std::pair<Texture*, TextureLayout>> textures)
{
	m_meshes.push_back({ vertexBuffer, createDescriptorSet(device, descriptorPool, ubos, textures) });
	
	return m_meshes.size() - 1;
}

void Renderer::cleanup(VkDevice device, VkDescriptorPool descriptorPool)
{
	for (int i(0); i < m_meshes.size(); ++i)
		vkFreeDescriptorSets(device, descriptorPool, 1, &m_meshes[i].second);
	vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
	destroyPipeline(device);
}

std::vector<std::pair<VertexBuffer, VkDescriptorSet>> Renderer::getMeshes()
{
	return m_meshes;
}

void Renderer::createDescriptorSetLayout(VkDevice device, std::vector<UniformBufferObjectLayout> uniformBufferObjectLayouts, std::vector<TextureLayout> textureLayouts)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (int i(0); i < uniformBufferObjectLayouts.size(); ++i)
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = uniformBufferObjectLayouts[i].binding;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = uniformBufferObjectLayouts[i].accessibility;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		bindings.push_back(uboLayoutBinding);
	}

	if(textureLayouts.size() > 0)
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = textureLayouts[0].binding;
		samplerLayoutBinding.descriptorCount = static_cast<uint32_t>(textureLayouts.size());
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = textureLayouts[0].accessibility;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		bindings.push_back(samplerLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Error : create descriptor set layout");
}

VkDescriptorSet Renderer::createDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool,
	std::vector<std::pair<UniformBufferObject*, UniformBufferObjectLayout>> ubos, std::vector<std::pair<Texture*, TextureLayout>> textures)
{
	VkDescriptorSetLayout layouts[] = { m_descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	VkDescriptorSet descriptorSet;
	VkResult res = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Error : allocate descriptor set");

	std::vector<VkWriteDescriptorSet> descriptorWrites;

	std::vector<VkDescriptorBufferInfo> bufferInfo(ubos.size());
	for (int i(0); i < ubos.size(); ++i)
	{
		bufferInfo[i].buffer = ubos[i].first->getUniformBuffer();
		bufferInfo[i].offset = 0;
		bufferInfo[i].range = ubos[i].first->getSize();

		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = ubos[i].second.binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo[i];
		descriptorWrite.pNext = NULL;

		descriptorWrites.push_back(descriptorWrite);
	}

	std::vector<VkDescriptorImageInfo> imageInfo(textures.size());
	for (int i(0); i < textures.size(); ++i)
	{
		imageInfo[i].imageLayout = textures[i].first->getImageLayout();
		imageInfo[i].imageView = textures[i].first->getImageView();
		imageInfo[i].sampler = textures[i].first->getSampler();
	}

	if (textures.size() > 0)
	{
		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = textures[0].second.binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = static_cast<uint32_t>(imageInfo.size());
		descriptorWrite.pImageInfo = imageInfo.data();
		descriptorWrite.pNext = NULL;

		descriptorWrites.push_back(descriptorWrite);
	}

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	return descriptorSet;
}
