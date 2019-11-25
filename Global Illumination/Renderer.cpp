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
	m_pipeline.initialize(device, renderPass, m_vertexShader, m_fragmentShader, m_vertexInputDescription, m_attributeInputDescription, extent, msaa, m_alphaBlending, &m_descriptorSetLayout);
}

int Renderer::addModel(Model* model)
{
	m_models.push_back(model);
	
	return m_models.size() - 1;
}

void Renderer::cleanup(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
	m_pipeline.cleanup(device);
}

std::vector<VertexBuffer> Renderer::getVertexBuffers()
{
	std::vector<VertexBuffer> vertexBuffers;

	for (int i(0); i < m_models.size(); ++i)
	{
		std::vector<VertexBuffer> modelVertexBuffers = m_models[i]->getVertexBuffers();
		vertexBuffers.insert(vertexBuffers.end(), modelVertexBuffers.begin(), modelVertexBuffers.end());
	}

	return vertexBuffers;
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

	for (int i(0); i < textureLayouts.size(); ++i)
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = textureLayouts[i].binding;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = textureLayouts[i].accessibility;
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
