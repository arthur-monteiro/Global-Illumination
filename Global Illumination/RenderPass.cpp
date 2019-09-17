#include "RenderPass.h"

RenderPass::~RenderPass()
{
	if (m_isInitialized)
		std::cout << "Render pass not destroyed !" << std::endl;
}

void RenderPass::initialize(Vulkan* vk, std::vector<VkExtent2D> extent, bool present, VkSampleCountFlagBits msaaSamples, bool colorAttachment, bool depthAttachment, VkImageLayout finalLayout)
{
	if (m_isInitialized)
		return;

	if (extent.size() == 0)
		throw std::runtime_error("Can't create RenderPass without extent");

	m_text = nullptr;
	m_useColorAttachment = colorAttachment;
	m_useDepthAttachment = depthAttachment;
	m_msaaSamples = msaaSamples;
	m_extent = !present ? extent : std::vector<VkExtent2D>(1, vk->getSwapChainExtend());

	m_format = vk->getSwapChainImageFormat();
	if (!present)
		m_format = VK_FORMAT_R32G32B32A32_SFLOAT;
	m_depthFormat = vk->findDepthFormat();
	if(!colorAttachment)
		m_depthFormat = VK_FORMAT_D16_UNORM;

	if(present)
		createRenderPass(vk->getDevice(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, colorAttachment, depthAttachment); 
	else
		createRenderPass(vk->getDevice(), finalLayout, colorAttachment, depthAttachment);
	createDescriptorPool(vk->getDevice());
	
	if(colorAttachment && msaaSamples != VK_SAMPLE_COUNT_1_BIT)
		createColorResources(vk, m_extent[0]);
#ifndef NDEBUG
	if (msaaSamples != VK_SAMPLE_COUNT_1_BIT && m_extent.size() > 1)
		std::cout << "Warning : Can't create MSAA pass with multiple frambuffers" << std::endl; // !! to be fixed
#endif // NDEBUG

	if (!present)
	{
		m_frameBuffers.resize(m_extent.size());
		m_commandBuffer.resize(m_extent.size());
		for(int i(0); i < m_extent.size(); ++i)
			m_frameBuffers[i] = vk->createFrameBuffer(extent[i], m_renderPass, m_msaaSamples, m_colorImageView, depthAttachment ? m_depthFormat : VK_FORMAT_UNDEFINED,
				colorAttachment ? m_format : VK_FORMAT_UNDEFINED);
	}
	else
		vk->createSwapchainFramebuffers(m_renderPass, m_msaaSamples, m_colorImageView);

	if (colorAttachment)
	{
		UniformBufferObject<UniformBufferObjectSingleVec> uboTemp;
		uboTemp.load(vk, { glm::vec4(0.0f) }, VK_SHADER_STAGE_FRAGMENT_BIT);
		m_textDescriptorSetLayout = createDescriptorSetLayout(vk->getDevice(), { &uboTemp }, 1);
		m_textPipeline.initialize(vk, &m_textDescriptorSetLayout, m_renderPass, { "Shaders/text/vert.spv", "", "Shaders/text/frag.spv" }, true, m_msaaSamples, { TextVertex::getBindingDescription() }, TextVertex::getAttributeDescriptions(), vk->getSwapChainExtend());
		uboTemp.cleanup(vk->getDevice());
	}

	m_useSwapChain = present;
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(vk->getDevice(), &semaphoreInfo, nullptr, &m_renderCompleteSemaphore) != VK_SUCCESS)
		throw std::runtime_error("Erreur : création de la sémaphores");

	//vk->setRenderFinishedLastRenderPassSemaphore(m_renderCompleteSemaphore);

	m_commandPool = vk->createCommandPool();

	m_isInitialized = true;
}

int RenderPass::addMesh(Vulkan * vk, std::vector<MeshRender> meshes, PipelineShaders pipelineShaders, int nbTexture, bool alphaBlending, int frameBufferID)
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
	pipeline.initialize(vk, &descriptorSetLayout, m_renderPass, pipelineShaders, alphaBlending, m_msaaSamples, { VertexPBR::getBindingDescription(0) },
		VertexPBR::getAttributeDescriptions(0), m_extent[frameBufferID]);
	meshesPipeline.pipeline = pipeline.getGraphicsPipeline();
	meshesPipeline.pipelineLayout = pipeline.getPipelineLayout();
	
	for (int i(0); i < meshes.size(); ++i)
	{
		for (int k(0); k < meshes[i].meshes.size(); ++k)
		{
			meshesPipeline.vertexBuffer.push_back(meshes[i].meshes[k]->getVertexBuffer());
			meshesPipeline.indexBuffer.push_back(meshes[i].meshes[k]->getIndexBuffer());
			meshesPipeline.nbIndices.push_back(meshes[i].meshes[k]->getNumIndices());

#ifndef NDEBUG
			if (meshes[i].meshes[k]->getImageView().size() + meshes[i].images.size() != nbTexture)
				std::cout << "Attention : le nombre de texture utilisés n'est pas égale au nombre de textures du mesh" << std::endl;
#endif // DEBUG

			std::vector<VkImageView> imageViewToAdd = meshes[i].meshes[k]->getImageView();
			for (int j(0); j < meshes[i].images.size(); ++j)
				imageViewToAdd.push_back(meshes[i].images[j].first);

			if (nbTexture <= meshes[i].images.size()) // additionnal image views are prioritized
			{
				imageViewToAdd.clear();
				for (int j(0); j < meshes[i].images.size(); ++j)
					imageViewToAdd.push_back(meshes[i].images[j].first);
			}

			std::vector<VkImageLayout> imageLayouts(imageViewToAdd.size());
			if (nbTexture > meshes[i].images.size())
			{
				for (int j(0); j < meshes[i].meshes[k]->getImageView().size(); ++j)
					imageLayouts[j] = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				for (int j(meshes[i].meshes[k]->getImageView().size()); j < imageViewToAdd.size(); ++j)
					imageLayouts[j] = m_meshes[i].images[j - meshes[i].meshes[k]->getImageView().size()].second;
			}
			else
			{
				for (int j(0); j < imageLayouts.size(); ++j)
					imageLayouts[j] = m_meshes[i].images[j].second;
			}

			VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), descriptorSetLayout,
				imageViewToAdd, meshes[i].meshes[k]->getSampler(), meshes[i].ubos, nbTexture, imageLayouts);
			meshesPipeline.descriptorSet.push_back(descriptorSet);
		}
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

#ifndef NDEBUG
	if (m_extent.size() > 1)
		std::cout << "Warning : Can't instanciate with multiple frambuffers, default is first framebuffer" << std::endl; // !! to be fixed
#endif // NDEBUG

	pipeline.initialize(vk, &descriptorSetLayout, m_renderPass, { vertPath, "", fragPath }, false, m_msaaSamples, { VertexPBR::getBindingDescription(0), ModelInstance::getBindingDescription(1) },
		attributeDescription, m_extent[0]);
	meshesPipelineInstanced.pipeline = pipeline.getGraphicsPipeline();
	meshesPipelineInstanced.pipelineLayout = pipeline.getPipelineLayout();

	for (int i(0); i < meshes.size(); ++i)
	{
		for (int k(0); k < meshes[i].meshes.size(); ++k)
		{
			meshesPipelineInstanced.vertexBuffer.push_back(meshes[i].meshes[k]->getVertexBuffer());
			meshesPipelineInstanced.indexBuffer.push_back(meshes[i].meshes[k]->getIndexBuffer());
			meshesPipelineInstanced.nbIndices.push_back(meshes[i].meshes[k]->getNumIndices());
			meshesPipelineInstanced.instanceBuffer.push_back(meshes[i].instance->getInstanceBuffer());

#ifndef NDEBUG
			if (meshes[i].meshes[k]->getImageView().size() != nbTexture)
				std::cout << "Attention : le nombre de texture utilisés n'est pas égale au nombre de textures du mesh" << std::endl;
#endif // DEBUG

			VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), descriptorSetLayout,
				meshes[i].meshes[k]->getImageView(), meshes[i].meshes[k]->getSampler(), meshes[i].ubos, nbTexture, { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }); // to correct
			meshesPipelineInstanced.descriptorSet.push_back(descriptorSet);
		}
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
				std::vector<VkImageView>(1, text->getImageView(i, j)), text->getSampler(), { text->getUbo(i) }, 1, { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
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

#ifndef NDEBUG
		if (m_extent.size() > 1)
			std::cout << "Warning : Can't draw menu with multiple frambuffers, default is first framebuffer" << std::endl;
#endif // NDEBUG

		Pipeline pipeline;
		pipeline.initialize(vk, &descriptorSetLayout, m_renderPass, { "Shaders/menuFullQuad/vert.spv", "", "Shaders/menuFullQuad/frag.spv" }, true, m_msaaSamples,
			{ VertexQuad::getBindingDescription(0) }, VertexQuad::getAttributeDescriptions(0), m_extent[0]);
		meshesPipeline.pipeline = pipeline.getGraphicsPipeline();
		meshesPipeline.pipelineLayout = pipeline.getPipelineLayout();

		meshesPipeline.vertexBuffer.push_back(quad->getVertexBuffer());
		meshesPipeline.indexBuffer.push_back(quad->getIndexBuffer());
		meshesPipeline.nbIndices.push_back(quad->getNumIndices());

		VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), descriptorSetLayout,
			std::vector<VkImageView>(), VK_NULL_HANDLE, std::vector<UboBase*>(), 0, {});
		meshesPipeline.descriptorSet.push_back(descriptorSet);

		m_meshesPipeline.push_back(meshesPipeline);
	}

	/* Item quads */
	{
		MeshPipeline meshesPipeline;

		std::vector<UboBase*> UBOs = menu->getUBOs();
		VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout(vk->getDevice(), { UBOs[0] }, 0);

		Pipeline pipeline;
		pipeline.initialize(vk, &descriptorSetLayout, m_renderPass, { "Shaders/menuItemQuad/vert.spv", "", "Shaders/menuItemQuad/frag.spv" }, true, m_msaaSamples,
			{ VertexQuad::getBindingDescription(0) }, VertexQuad::getAttributeDescriptions(0), m_extent[0]);
		meshesPipeline.pipeline = pipeline.getGraphicsPipeline();
		meshesPipeline.pipelineLayout = pipeline.getPipelineLayout();

		for (int i(0); i < UBOs.size(); ++i)
		{
			meshesPipeline.vertexBuffer.push_back(quad->getVertexBuffer());
			meshesPipeline.indexBuffer.push_back(quad->getIndexBuffer());
			meshesPipeline.nbIndices.push_back(quad->getNumIndices());

			VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), descriptorSetLayout,
				std::vector<VkImageView>(), VK_NULL_HANDLE, { UBOs[i] }, 0, {});
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
				std::vector<VkImageView>(1, text.getImageView(i, j)), text.getSampler(), { text.getUbo(i) }, 1, { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
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
		pipeline.initialize(vk, &m_menuOptionImageDescriptorLayout, m_renderPass, { "Shaders/menuOptionImageQuad/vert.spv", "", "Shaders/menuOptionImageQuad/frag.spv" }, false, m_msaaSamples,
			{ VertexQuadTextured::getBindingDescription(0) }, VertexQuadTextured::getAttributeDescriptions(0), m_extent[0]);
		meshPipeline.pipeline = pipeline.getGraphicsPipeline();
		meshPipeline.pipelineLayout = pipeline.getPipelineLayout();

		Mesh2DTextured* quadImageOption = menu->getQuadImageOption();

		meshPipeline.vertexBuffer.push_back(quadImageOption->getVertexBuffer());
		meshPipeline.indexBuffer.push_back(quadImageOption->getIndexBuffer());
		meshPipeline.nbIndices.push_back(quadImageOption->getNumIndices());

		std::vector<VkImageView> imageViews = {};

		m_menuOptionImageSampler = quadImageOption->getSampler();
		VkDescriptorSet descriptorSet = createDescriptorSet(vk->getDevice(), m_menuOptionImageDescriptorLayout,
			imageViews, m_menuOptionImageSampler, { }, imageViews.size(), {});
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
		{ imageView }, m_menuOptionImageSampler, { }, 1, { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
	
	m_meshesPipeline[m_menuOptionImageMeshPipelineID].descriptorSet[0] = descriptorSet;
	m_drawOptionImage = true;

	recordDraw(vk);
}

void RenderPass::recordDraw(Vulkan * vk, std::vector<Operation> operations)
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
	else fillCommandBuffer(vk, operations);
}

void RenderPass::drawCall(Vulkan * vk)
{
	if (m_text && m_text->needUpdate() != -1)
	{
		m_meshesPipeline[m_textID[m_text->needUpdate()]].free(vk->getDevice(), m_descriptorPool);

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
				{ m_text->getUbo(m_text->needUpdate()) }, 1, { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
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
	if (!m_isInitialized)
		return;

	if (m_colorImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(vk->getDevice(), m_colorImageView, nullptr);
		vkDestroyImage(vk->getDevice(), m_colorImage, nullptr);
		vkFreeMemory(vk->getDevice(), m_colorImageMemory, nullptr);
	}
	m_colorImageView = VK_NULL_HANDLE;

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

	m_isInitialized = false;
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
	colorAttachment.finalLayout = m_msaaSamples != VK_SAMPLE_COUNT_1_BIT ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : finalLayout;

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
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, VK_IMAGE_LAYOUT_PREINITIALIZED, m_colorImage, m_colorImageMemory);
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
	VkSampler sampler, std::vector<UboBase*> uniformBuffer, int nbTexture, std::vector<VkImageLayout> imageLayouts)
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
		imageInfo[i - uniformBuffer.size()].imageLayout = imageLayouts[i - uniformBuffer.size()]; // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
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

void RenderPass::fillCommandBuffer(Vulkan * vk, std::vector<Operation> operations)
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
		renderPassInfo.renderArea.extent = m_extent[i];

		std::vector<VkClearValue> clearValues = {};
		if (m_useColorAttachment && m_useDepthAttachment)
			clearValues = { { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f } };
		else if (m_useDepthAttachment)
			clearValues = { { 1.0f } };

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

		for (int j(0); j < operations.size(); ++j)
		{
			if (operations[j].type == OPERATION_TYPE_BLIT)
			{
				VkImageMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = operations[j].dstBlitImage;
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;

				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

				vkCmdPipelineBarrier(
					m_commandBuffer[i],
					sourceStage, destinationStage,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);

				VkImageBlit blit = {};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { (int32_t)m_extent[i].width, (int32_t)m_extent[i].height, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = 0;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { (int32_t)operations[j].dstBlitExtent.width, (int32_t)operations[j].dstBlitExtent.height, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = 0;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				vkCmdBlitImage(m_commandBuffer[i],
					m_frameBuffers[i].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					operations[j].dstBlitImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit,
					VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;

				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			
				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

				vkCmdPipelineBarrier(
					m_commandBuffer[i],
					sourceStage, destinationStage,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);
			}
		}

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
	submitInfo.waitSemaphoreCount = m_needToWaitSemaphores.size();
	submitInfo.pWaitSemaphores = m_needToWaitSemaphores.data();
	submitInfo.pWaitDstStageMask = m_needToWaitStages.data();

	if (vkQueueSubmit(vk->getGraphicalQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("Erreur : draw command");
}

void RenderPass::setSemaphoreToWait(VkDevice device, std::vector<Semaphore> semaphores)
{
	/*for (int i(0); i < m_needToWaitSemaphores.size(); ++i)
		vkDestroySemaphore(device, m_needToWaitSemaphores[i], nullptr);*/
	m_needToWaitSemaphores.clear();
	m_needToWaitStages.clear();

    for(int i(0); i < semaphores.size(); ++i)
    {
        m_needToWaitSemaphores.push_back(semaphores[i].semaphore);
        m_needToWaitStages.push_back(semaphores[i].stage);
    }
}
