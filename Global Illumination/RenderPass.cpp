#include "RenderPass.h"

RenderPass::~RenderPass()
{
}

bool RenderPass::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<Attachment> attachments, std::vector<VkExtent2D> extents)
{
	if (attachments.size() == 0)
		throw std::runtime_error("Can't create RenderPass without attachment");

	m_renderPass = createRenderPass(device, attachments);
	m_command.initialize(device, physicalDevice, surface);

	return true;
}

bool RenderPass::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<Attachment> attachments, std::vector<Image*> images)
{
	if (attachments.size() == 0)
		throw std::runtime_error("Can't create RenderPass without attachment");

	m_renderPass = createRenderPass(device, attachments);
	m_command.initialize(device, physicalDevice, surface);

	m_framebuffers.resize(images.size());
	for (int i(0); i < images.size(); ++i)
		m_framebuffers[i].initialize(device, physicalDevice, m_renderPass, images[i], attachments);

	return true;
}

void RenderPass::submit(VkDevice device)
{
}

VkRenderPass RenderPass::createRenderPass(VkDevice device, std::vector<Attachment> attachments)
{
	std::vector<VkAttachmentReference> colorAttachmentRefs;
	VkAttachmentReference depthAttachmentRef;
	std::vector<VkAttachmentReference> resolveAttachmentRefs;

	// Attachment descriptions
	std::vector<VkAttachmentDescription> attachmentDescriptions(attachments.size());
	for (int i(0); i < attachments.size(); ++i)
	{
		attachmentDescriptions[i].format = attachments[i].getFormat();
		attachmentDescriptions[i].samples = attachments[i].getSampleCount();
		attachmentDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[i].storeOp = attachments[i].getStoreOperation();
		attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[i].finalLayout = attachments[i].getFinalLayout();

		VkAttachmentReference ref;
		ref.attachment = i;

		if (attachments[i].getUsageType() == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		{
			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachmentRefs.push_back(ref);
		}
		else if (attachments[i].getUsageType() == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachmentRef = ref;
		}
		else if (attachments[i].getUsageType() == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
		{
			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			resolveAttachmentRefs.push_back(ref);
		}
	}

	// Subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = colorAttachmentRefs.size();
	subpass.pColorAttachments = colorAttachmentRefs.data();
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	if(resolveAttachmentRefs.size() > 0)
		subpass.pResolveAttachments = resolveAttachmentRefs.data();

	// Dependencies
	std::vector<VkSubpassDependency> dependencies(colorAttachmentRefs.size() > 0 ? 1 : 2);
	if (colorAttachmentRefs.size() > 0)
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

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassInfo.pAttachments = attachmentDescriptions.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	VkRenderPass renderPassToReturn;
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPassToReturn) != VK_SUCCESS)
		throw std::runtime_error("Error : create render pass");

	return renderPassToReturn;
}
