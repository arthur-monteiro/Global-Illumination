#include "RenderPass.h"

RenderPass::~RenderPass()
{
	if (!m_isDestroyed)
		std::cout << "Render pass not destroyed !" << std::endl;
}

void RenderPass::initialize(Vulkan* vk, bool createFrameBuffer, VkExtent2D extent, bool present, VkSampleCountFlagBits msaaSamples, int nbFramebuffer, bool colorAttachment, bool depthAttachment)
{
	m_text = nullptr;
	m_msaaSamples = msaaSamples;
	m_extent = createFrameBuffer ? extent : vk->getSwapChainExtend();

	m_format = vk->getSwapChainImageFormat();
	if (createFrameBuffer)
		m_format = VK_FORMAT_R32G32B32A32_SFLOAT;
	m_depthFormat = vk->findDepthFormat();
	if(!colorAttachment)
		m_depthFormat = VK_FORMAT_D16_UNORM;

	if(present)
		createRenderPass(vk->getDevice(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, colorAttachment, depthAttachment);
	else
		createRenderPass(vk->getDevice(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, colorAttachment, depthAttachment);
	createDescriptorPool(vk->getDevice());
	
	if(colorAttachment)
		createColorResources(vk, m_extent);
	if (createFrameBuffer)
	{
		m_frameBuffers.resize(nbFramebuffer);
		m_commandBuffer.resize(nbFramebuffer);
		for(int i(0); i < nbFramebuffer; ++i)
			m_frameBuffers[i] = vk->createFrameBuffer(extent, m_renderPass, m_msaaSamples, m_colorImageView, depthAttachment ? m_depthFormat : VK_FORMAT_UNDEFINED,
				colorAttachment ? m_format : VK_FORMAT_UNDEFINED);
	}
	else
		vk->createSwapchainFramebuffers(m_renderPass, m_msaaSamples, m_colorImageView);

	if (colorAttachment)
	{
		UniformBufferObject<UniformBufferObjectSingleVec> uboTemp;
		uboTemp.load(vk, { glm::vec4(0.0f) }, VK_SHADER_STAGE_FRAGMENT_BIT);
		m_textDescriptorSetLayout = createDescriptorSetLayout(vk->getDevice(), { &uboTemp }, 1);
		m_textPipeline.initialize(vk, &m_textDescriptorSetLayout, m_renderPass, "Shaders/text/vert.spv",
			"Shaders/text/frag.spv", true, m_msaaSamples, { TextVertex::getBindingDescription() }, TextVertex::getAttributeDescriptions(), vk->getSwapChainExtend());
		uboTemp.cleanup(vk->getDevice());
	}

	m_useSwapChain = !createFrameBuffer;
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(vk->getDevice(), &semaphoreInfo, nullptr, &m_renderCompleteSemaphore) != VK_SUCCESS)
		throw std::runtime_error("Erreur : création de la sémaphores");

	vk->setRenderFinishedLastRenderPassSemaphore(m_renderCompleteSemaphore);

	m_commandPool = vk->createCommandPool();
}

int RenderPass::addMesh(Vulkan * vk, std::vector<MeshRender> meshes, std::string vertPath, std::string fragPath, int nbTexture, bool alphaBlending, int frameBufferID)
{
	/* Ici tous les meshes sont rendus avec les mêmes shaders */
	for (int i(0); i < meshes.size(); ++i)
	{
		m_meshes.push_back(meshes[i]);
	}
	MeshPipeline meshesPipeline;

	// Tous les meshes doivent avoir la même définition d'ubo
	VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout(vk->getDevice(), meshes[0].ubos, nbTexture);

	Pipeline pipeline;
	pipeline.initialize(vk, &descriptorSetLayout, m_renderPass, vertPath, fragPath, alphaBlending, m_msaaSamples, { VertexPBR::getBindingDescription(0) }, VertexPBR::getAttributeDescriptions(0), m_extent);
	meshesPipeline.pipeline = pipeline.getGraphicsPipeline();
	meshesPipeline.pipelineLayout = pipeline.getPipelineLayout();
	
	for (int i(0); i < meshes.size(); ++i)
	{
		meshesPipeline.vertexBuffer.push_back(meshes[i].mesh->getVertexBuffer());
		meshesPipeline.indexBuffer.push_back(meshes[i].mesh->getIndexBuffer());
		meshesPipeline.nbIndices.push_back(meshes[i].mesh->getNumIndices());

#ifndef NDEBUG
		if (meshes[i].mesh->getImageView().size() + meshes[i].imageViews.size() != nbTexture)
			std::cout << "Attention : le nombre de texture utilisés n'est pas égale au nombre de textures du mesh" << std::endl;
#endif // DEBUG

		std::vector<VkImageView> imageViewToAdd = meshes[i].mesh->getImageView();
		for (int j(0); j < meshes[i].imageViews.size(); ++j)
			imageViewToAdd.push_back(meshes[i].imageViews[j]);

		VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), descriptorSetLayout,
			imageViewToAdd, meshes[i].mesh->getSampler(), meshes[i].ubos, nbTexture, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
		meshesPipeline.descriptorSet.push_back(descriptorSet);
	}

	meshesPipeline.frameBufferID = frameBufferID;
	
	m_meshesPipeline.push_back(meshesPipeline);

	return (int)m_meshesPipeline.size() - 1;
}

int RenderPass::addMeshInstanced(Vulkan* vk, std::vector<MeshRender> meshes, std::string vertPath, std::string fragPath, int nbTexture)
{
	MeshPipeline meshesPipelineInstanced;

	VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout(vk->getDevice(), meshes[0].ubos, nbTexture);

	Pipeline pipeline;
	std::vector<VkVertexInputAttributeDescription> attributeDescription = VertexPBR::getAttributeDescriptions(0);
	std::vector<VkVertexInputAttributeDescription> instanceAttributeDescription = ModelInstance::getAttributeDescriptions(1, 4);
	for (int i(0); i < instanceAttributeDescription.size(); ++i)
	{
		attributeDescription.push_back(instanceAttributeDescription[i]);
	}
	pipeline.initialize(vk, &descriptorSetLayout, m_renderPass, vertPath, fragPath, false, m_msaaSamples, { VertexPBR::getBindingDescription(0), ModelInstance::getBindingDescription(1) }, 
		attributeDescription, m_extent);
	meshesPipelineInstanced.pipeline = pipeline.getGraphicsPipeline();
	meshesPipelineInstanced.pipelineLayout = pipeline.getPipelineLayout();

	for (int i(0); i < meshes.size(); ++i)
	{
		meshesPipelineInstanced.vertexBuffer.push_back(meshes[i].mesh->getVertexBuffer());
		meshesPipelineInstanced.indexBuffer.push_back(meshes[i].mesh->getIndexBuffer());
		meshesPipelineInstanced.nbIndices.push_back(meshes[i].mesh->getNumIndices());
		meshesPipelineInstanced.instanceBuffer.push_back(meshes[i].instance->getInstanceBuffer());

#ifndef NDEBUG
		if (meshes[i].mesh->getImageView().size() != nbTexture)
			std::cout << "Attention : le nombre de texture utilisés n'est pas égale au nombre de textures du mesh" << std::endl;
#endif // DEBUG

		VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), descriptorSetLayout,
			meshes[i].mesh->getImageView(), meshes[i].mesh->getSampler(), meshes[i].ubos, nbTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		meshesPipelineInstanced.descriptorSet.push_back(descriptorSet);
	}

	m_meshesPipeline.push_back(meshesPipelineInstanced);

	return (int)m_meshesPipeline.size() - 1;
}

int RenderPass::addText(Vulkan * vk, Text * text)
{
	m_text = text;

	int nbText = text->getNbTexts();
	for (int i = 0; i < nbText; ++i)
	{
		m_textID.push_back(m_meshesPipeline.size());
		MeshPipeline meshPipeline;
		meshPipeline.pipeline = m_textPipeline.getGraphicsPipeline();
		meshPipeline.pipelineLayout = m_textPipeline.getPipelineLayout();

		for(int j = 0; j < text->getNbCharacters(i); ++j)
		{
			meshPipeline.vertexBuffer.push_back(text->getVertexBuffer(i, j));
			meshPipeline.indexBuffer.push_back(text->getIndexBuffer());
			meshPipeline.nbIndices.push_back(6);

			VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), m_textDescriptorSetLayout,
				std::vector<VkImageView>(1, text->getImageView(i, j)), text->getSampler(), { text->getUbo(i) }, 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			meshPipeline.descriptorSet.push_back(descriptorSet);
		}

		m_meshesPipeline.push_back(meshPipeline);
	}

	return 0;
}

void RenderPass::addMenu(Vulkan* vk, Menu * menu)
{
	Mesh2D* quad = menu->getQuadFull();

	/* Full quad */
	{
		m_menuMeshesPipelineIDs.push_back(m_meshesPipeline.size());

		MeshPipeline meshesPipeline;
		VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout(vk->getDevice(), std::vector<UboBase*>(), 0);

		Pipeline pipeline;
		pipeline.initialize(vk, &descriptorSetLayout, m_renderPass, "Shaders/menuFullQuad/vert.spv", "Shaders/menuFullQuad/frag.spv", true, m_msaaSamples,
			{ VertexQuad::getBindingDescription(0) }, VertexQuad::getAttributeDescriptions(0), m_extent);
		meshesPipeline.pipeline = pipeline.getGraphicsPipeline();
		meshesPipeline.pipelineLayout = pipeline.getPipelineLayout();

		meshesPipeline.vertexBuffer.push_back(quad->getVertexBuffer());
		meshesPipeline.indexBuffer.push_back(quad->getIndexBuffer());
		meshesPipeline.nbIndices.push_back(quad->getNumIndices());

		VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), descriptorSetLayout,
			std::vector<VkImageView>(), VK_NULL_HANDLE, std::vector<UboBase*>(), 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		meshesPipeline.descriptorSet.push_back(descriptorSet);

		m_meshesPipeline.push_back(meshesPipeline);
	}

	/* Item quads */
	{
		MeshPipeline meshesPipeline;

		std::vector<UboBase*> UBOs = menu->getUBOs();
		VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout(vk->getDevice(), { UBOs[0] }, 0);

		Pipeline pipeline;
		pipeline.initialize(vk, &descriptorSetLayout, m_renderPass, "Shaders/menuItemQuad/vert.spv", "Shaders/menuItemQuad/frag.spv", true, m_msaaSamples,
			{ VertexQuad::getBindingDescription(0) }, VertexQuad::getAttributeDescriptions(0), m_extent);
		meshesPipeline.pipeline = pipeline.getGraphicsPipeline();
		meshesPipeline.pipelineLayout = pipeline.getPipelineLayout();

		for (int i(0); i < UBOs.size(); ++i)
		{
			meshesPipeline.vertexBuffer.push_back(quad->getVertexBuffer());
			meshesPipeline.indexBuffer.push_back(quad->getIndexBuffer());
			meshesPipeline.nbIndices.push_back(quad->getNumIndices());

			VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), descriptorSetLayout,
				std::vector<VkImageView>(), VK_NULL_HANDLE, { UBOs[i] }, 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			meshesPipeline.descriptorSet.push_back(descriptorSet);
		}

		m_menuMeshesPipelineIDs.push_back(m_meshesPipeline.size());
		m_meshesPipeline.push_back(meshesPipeline);
	}

	/* Text */
	Text text = menu->getText();

	int nbText = text.getNbTexts();
	for (int i = 0; i < nbText; ++i)
	{
		MeshPipeline meshPipeline;
		meshPipeline.pipeline = m_textPipeline.getGraphicsPipeline();
		meshPipeline.pipelineLayout = m_textPipeline.getPipelineLayout();

		for (int j = 0; j < text.getNbCharacters(i); ++j)
		{
			meshPipeline.vertexBuffer.push_back(text.getVertexBuffer(i, j));
			meshPipeline.indexBuffer.push_back(text.getIndexBuffer());
			meshPipeline.nbIndices.push_back(6);

			VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), m_textDescriptorSetLayout,
				std::vector<VkImageView>(1, text.getImageView(i, j)), text.getSampler(), { text.getUbo(i) }, 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			meshPipeline.descriptorSet.push_back(descriptorSet);
		}

		m_menuMeshesPipelineIDs.push_back(m_meshesPipeline.size());
		m_meshesPipeline.push_back(meshPipeline);
	}

	/* Option quad */
	{
		MeshPipeline meshPipeline;
		m_menuOptionImageDescriptorLayout = createDescriptorSetLayout(vk->getDevice(), { }, 1);

		Pipeline pipeline;
		pipeline.initialize(vk, &m_menuOptionImageDescriptorLayout, m_renderPass, "Shaders/menuOptionImageQuad/vert.spv", "Shaders/menuOptionImageQuad/frag.spv", false, m_msaaSamples,
			{ VertexQuadTextured::getBindingDescription(0) }, VertexQuadTextured::getAttributeDescriptions(0), m_extent);
		meshPipeline.pipeline = pipeline.getGraphicsPipeline();
		meshPipeline.pipelineLayout = pipeline.getPipelineLayout();

		Mesh2DTextured* quadImageOption = menu->getQuadImageOption();

		meshPipeline.vertexBuffer.push_back(quadImageOption->getVertexBuffer());
		meshPipeline.indexBuffer.push_back(quadImageOption->getIndexBuffer());
		meshPipeline.nbIndices.push_back(quadImageOption->getNumIndices());

		std::vector<VkImageView> imageViews = {};

		m_menuOptionImageSampler = quadImageOption->getSampler();
		VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), m_menuOptionImageDescriptorLayout,
			imageViews, m_menuOptionImageSampler, { }, imageViews.size(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		meshPipeline.descriptorSet.push_back(descriptorSet);

		m_menuMeshesPipelineIDs.push_back(m_meshesPipeline.size());
		m_menuOptionImageMeshPipelineID = m_meshesPipeline.size();
		m_meshesPipeline.push_back(meshPipeline);
	}
}

void RenderPass::updateImageViewMenuItemOption(Vulkan* vk, VkImageView imageView)
{
	if (imageView == VK_NULL_HANDLE)
	{
		m_drawOptionImage = false;
		recordDraw(vk);
		return;
	}

	VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), m_menuOptionImageDescriptorLayout,
		{ imageView }, m_menuOptionImageSampler, { }, 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
	m_meshesPipeline[m_menuOptionImageMeshPipelineID].descriptorSet[0] = descriptorSet;
	m_drawOptionImage = true;

	recordDraw(vk);
}

/*int RenderPass::addPointLight(Vulkan * vk, glm::vec3 position, glm::vec3 color)
{
	m_uboLights.pointLightsPositions[m_uboLights.nbPointLights] = glm::vec4(position, 1.0f);
	m_uboLights.pointLightsColors[m_uboLights.nbPointLights] = glm::vec4(color, 1.0f);
	m_uboLights.nbPointLights++;

	void* data;
	vkMapMemory(vk->getDevice(), m_uboLights.uniformBufferMemory, 0, sizeof(m_uboLights), 0, &data);
		memcpy(data, &m_uboLights, sizeof(m_uboLights));
	vkUnmapMemory(vk->getDevice(), m_uboLights.uniformBufferMemory);

	return m_uboLights.nbPointLights - 1;
}

int RenderPass::addDirLight(Vulkan* vk, glm::vec3 direction, glm::vec3 color)
{
	m_uboLights.dirLightsDirections[m_uboLights.nbDirLights] = glm::vec4(direction, 1.0f);
	m_uboLights.dirLightsColors[m_uboLights.nbDirLights] = glm::vec4(color, 1.0f);
	m_uboLights.nbDirLights++;

	void* data;
	vkMapMemory(vk->getDevice(), m_uboLights.uniformBufferMemory, 0, sizeof(m_uboLights), 0, &data);
	memcpy(data, &m_uboLights, sizeof(m_uboLights));
	vkUnmapMemory(vk->getDevice(), m_uboLights.uniformBufferMemory);

	return m_uboLights.nbDirLights - 1;
}*/

void RenderPass::recordDraw(Vulkan * vk)
{
	std::vector<MeshPipeline> meshesToRender;
	for (int i(0); i < m_meshesPipeline.size(); ++i)
	{
		if (!m_drawOptionImage && m_menuOptionImageMeshPipelineID == i)
			continue;
		if (!m_drawMenu)
		{
			bool draw = true;
			for (int k(0); k < m_menuMeshesPipelineIDs.size(); ++k)
				if (i == m_menuMeshesPipelineIDs[k])
				{
					draw = false;
					break;
				}
			if (!draw)
				continue;
		}
		if (!m_drawText)
		{
			bool draw = true;
			for (int k(0); k < m_textID.size(); ++k)
				if (i == m_textID[k])
				{
					draw = false;
					break;
				}
			if (!draw)
				continue;
		}
		meshesToRender.push_back(m_meshesPipeline[i]);
	}
	if(m_useSwapChain) vk->fillCommandBuffer(m_renderPass, meshesToRender);
	else fillCommandBuffer(vk);
}

/*void RenderPass::updateUniformBuffer(Vulkan * vk, int meshID)
{
	m_camera.update(vk->getWindow());

	for (int i = 0; i < m_meshes.size(); ++i)
	{
		if (meshID != -1)
			i = meshID;

		UniformBufferObjectMatrices ubo = {};
		ubo.matrix = m_meshes[i]->GetModelMatrix();
		if (meshID != -1)
			ubo.view = glm::mat4(glm::mat3(m_camera.getViewMatrix()));
		else
			ubo.view = m_camera.getViewMatrix();
		ubo.proj = glm::perspective(glm::radians(45.0f), vk->getSwapChainExtend().width / (float)vk->getSwapChainExtend().height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(vk->getDevice(), m_ubosMatrices[i].uniformBufferMemory, 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(vk->getDevice(), m_ubosMatrices[i].uniformBufferMemory);

		if (meshID != -1)
			break;
	}

	m_uboLights.camPos = glm::vec4(m_camera.getPosition(), 1.0f);

	void* data;
	vkMapMemory(vk->getDevice(), m_uboLights.uniformBufferMemory, 0, sizeof(m_uboLights), 0, &data);
		memcpy(data, &m_uboLights, sizeof(m_uboLights));
	vkUnmapMemory(vk->getDevice(), m_uboLights.uniformBufferMemory);
}*/

void RenderPass::drawCall(Vulkan * vk)
{
	if (m_text && m_text->needUpdate() != -1)
	{
		m_meshesPipeline[m_textID[m_text->needUpdate()]].free(vk->getDevice(), m_descriptorPool);
		//m_meshesPipeline.erase(m_meshesPipeline.begin() + m_textID[m_text->needUpdate()]);

		//m_textID[m_text->needUpdate()] = m_meshesPipeline.size();
		MeshPipeline meshPipeline;
		meshPipeline.pipeline = m_textPipeline.getGraphicsPipeline();
		meshPipeline.pipelineLayout = m_textPipeline.getPipelineLayout();

		for (int j = 0; j < m_text->getNbCharacters(m_text->needUpdate()); ++j)
		{
			meshPipeline.vertexBuffer.push_back(m_text->getVertexBuffer(m_text->needUpdate(), j));
			meshPipeline.indexBuffer.push_back(m_text->getIndexBuffer());
			meshPipeline.nbIndices.push_back(6);

			VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), m_textDescriptorSetLayout,
				std::vector<VkImageView>(1, m_text->getImageView(m_text->needUpdate(), j)), m_text->getSampler(),
				{ m_text->getUbo(m_text->needUpdate()) }, 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			meshPipeline.descriptorSet.push_back(descriptorSet);
		}

		m_meshesPipeline[m_textID[m_text->needUpdate()]] = meshPipeline;

		recordDraw(vk);

		m_text->updateDone();
	}
	
	if(m_useSwapChain)
		vk->drawFrame();
	else 
		drawFrame(vk);
}

void RenderPass::cleanup(Vulkan * vk)
{
	if (m_colorImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(vk->getDevice(), m_colorImageView, nullptr);
		vkDestroyImage(vk->getDevice(), m_colorImage, nullptr);
		vkFreeMemory(vk->getDevice(), m_colorImageMemory, nullptr);
	}

	for (int i(0); i < m_meshesPipeline.size(); ++i)
		m_meshesPipeline[i].free(vk->getDevice(), m_descriptorPool, true); // ne détruit pas les ressources

	for (int i(0); i < m_frameBuffers.size(); ++i)
		m_frameBuffers[i].free(vk->getDevice());

	m_meshes.clear();

	m_meshesPipeline.clear();
	m_textID.clear();
	m_menuMeshesPipelineIDs.clear();
	m_menuOptionImageMeshPipelineID = -1;
	m_drawOptionImage = false;

	vkDestroyDescriptorPool(vk->getDevice(), m_descriptorPool, nullptr);
	vkDestroyRenderPass(vk->getDevice(), m_renderPass, nullptr);

	m_isDestroyed = true;
}

void RenderPass::createRenderPass(VkDevice device, VkImageLayout finalLayout, bool useColorAttachment, bool useDepthAttachment)
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_format;
	colorAttachment.samples = m_msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = m_depthFormat;
	depthAttachment.samples = m_msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	if (useColorAttachment)
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	else 
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if(useColorAttachment)
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	else
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = useColorAttachment ? 1 : 0;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = m_format;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = finalLayout;

	VkAttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentResolveRef.attachment = useDepthAttachment ? 2 : 1;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = useColorAttachment ? 1 : 0;
	if(useColorAttachment)
		subpass.pColorAttachments = &colorAttachmentRef;
	if(useDepthAttachment)
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
	if(m_msaaSamples != VK_SAMPLE_COUNT_1_BIT)
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

	std::vector<VkSubpassDependency> dependencies(useColorAttachment ? 1 : 2);
	if (useColorAttachment)
	{
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	else
	{
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	}

	std::vector<VkAttachmentDescription> attachments;
	if(useColorAttachment && m_msaaSamples != VK_SAMPLE_COUNT_1_BIT)
		attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
	else if(useColorAttachment)
		attachments = { colorAttachment, depthAttachment };
	else 
		attachments = { depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
		throw std::runtime_error("Erreur : render pass");
}

void RenderPass::createColorResources(Vulkan* vk, VkExtent2D extent)
{
	VkFormat colorFormat = m_format;

	vk->createImage(extent.width, extent.height, 1, m_msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, m_colorImage, m_colorImageMemory);
	m_colorImageView = vk->createImageView(m_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_VIEW_TYPE_2D);

	vk->transitionImageLayout(m_colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1);
}

VkDescriptorSetLayout RenderPass::createDescriptorSetLayout(VkDevice device, std::vector<UboBase*> uniformBuffers, int nbTexture)
{
	int i = 0;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (; i < uniformBuffers.size(); ++i)
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = i;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = uniformBuffers[i]->getAccessibility();
		uboLayoutBinding.pImmutableSamplers = nullptr;

		bindings.push_back(uboLayoutBinding);
	}

	for (; i < nbTexture + uniformBuffers.size(); ++i)
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = i;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		bindings.push_back(samplerLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Erreur : descriptor set layout");

	return descriptorSetLayout;
}

void RenderPass::createDescriptorPool(VkDevice device)
{
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1024;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1024;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1024;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Erreur : création du descriptor pool");
}

VkDescriptorSet RenderPass::createDescriptorSet(VkDevice device, VkDescriptorSetLayout decriptorSetLayout, std::vector<VkImageView> imageView,
	VkSampler sampler, std::vector<UboBase*> uniformBuffer, int nbTexture, VkImageLayout imageLayout)
{
	VkDescriptorSetLayout layouts[] = { decriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	VkDescriptorSet descriptorSet;
	VkResult res = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Erreur : allocation descriptor set");

	std::vector<VkWriteDescriptorSet> descriptorWrites;

	int i = 0;
	std::vector<VkDescriptorBufferInfo> bufferInfo(uniformBuffer.size()); // ne doit pas être détruit avant l'appel de vkUpdateDescriptorSets
	for(; i < uniformBuffer.size(); ++i)
	{
		bufferInfo[i].buffer = uniformBuffer[i]->getUniformBuffer();
		bufferInfo[i].offset = 0;
		bufferInfo[i].range = uniformBuffer[i]->getSize();

		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = i;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo[i];
		descriptorWrite.pNext = NULL;

		descriptorWrites.push_back(descriptorWrite);
	}

	std::vector<VkDescriptorImageInfo> imageInfo(nbTexture);
	for(; i < uniformBuffer.size() + nbTexture; ++i)
	{
		imageInfo[i - uniformBuffer.size()].imageLayout = imageLayout; // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		imageInfo[i - uniformBuffer.size()].imageView = imageView[i - uniformBuffer.size()];
		imageInfo[i - uniformBuffer.size()].sampler = sampler;

		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = i;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo[i - uniformBuffer.size()];
		descriptorWrite.pNext = NULL;

		descriptorWrites.push_back(descriptorWrite);
	}
	
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	return descriptorSet;
}

void RenderPass::fillCommandBuffer(Vulkan * vk)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_commandBuffer.size();

	if (vkAllocateCommandBuffers(vk->getDevice(), &allocInfo, m_commandBuffer.data()) != VK_SUCCESS)
		throw std::runtime_error("Erreur : allocation des command buffers");

	for (int i(0); i < m_frameBuffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(m_commandBuffer[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass;
		renderPassInfo.framebuffer = m_frameBuffers[i].framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_extent;

		std::array<VkClearValue, 1> clearValues = {};
		//clearValues[0].color = { 0.0f, 0.0f, 1.0f, 1.0f };
		clearValues[0].depthStencil = { 1.0f };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_commandBuffer[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		for (int j = 0; j < m_meshesPipeline.size(); ++j)
		{
			if (m_meshesPipeline[j].frameBufferID != i)
				continue;

			if (!m_drawMenu)
			{
				bool draw = true;
				for (int k(0); k < m_menuMeshesPipelineIDs.size(); ++k)
					if (j == m_menuMeshesPipelineIDs[k])
					{
						draw = false;
						break;
					}
				if (!draw)
					continue;
			}

			for (int k(0); k < m_meshesPipeline[j].vertexBuffer.size(); ++k)
			{
				vkCmdBindPipeline(m_commandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_meshesPipeline[j].pipeline);

				VkBuffer vertexBuffers[] = { m_meshesPipeline[j].vertexBuffer[k] };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(m_commandBuffer[i], 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(m_commandBuffer[i], m_meshesPipeline[j].indexBuffer[k], 0, VK_INDEX_TYPE_UINT32);

				vkCmdBindDescriptorSets(m_commandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_meshesPipeline[j].pipelineLayout, 0, 1, &m_meshesPipeline[j].descriptorSet[k], 0, nullptr);

				vkCmdDrawIndexed(m_commandBuffer[i], m_meshesPipeline[j].nbIndices[k], 1, 0, 0, 0);
			}
		}

		vkCmdEndRenderPass(m_commandBuffer[i]);

		if (vkEndCommandBuffer(m_commandBuffer[i]) != VK_SUCCESS)
			throw std::runtime_error("Erreur : record command buffer");
	}
}

void RenderPass::drawFrame(Vulkan * vk)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_renderCompleteSemaphore; // renderPass
	submitInfo.commandBufferCount = m_commandBuffer.size();
	submitInfo.pCommandBuffers = m_commandBuffer.data();
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
	if (m_firstDraw)
	{
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = NULL;

		m_firstDraw = false;
	}

	if (vkQueueSubmit(vk->getGraphicalQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("Erreur : draw command");
}
