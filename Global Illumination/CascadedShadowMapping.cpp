#include "CascadedShadowMapping.h"

void CascadedShadowMapping::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkExtent2D extent, ModelPBR* model, glm::vec3 sunDir,
	float cameraNear, float cameraFar)
{
	std::vector<VkExtent2D> shadowMapExtents = { { 2048, 2048 }, { 2048, 2048 }, { 1024, 1024 }, { 1024, 1024 } };

	// Depth passes
	for (int i(0); i < m_depthPasses.size(); ++i) // for each cascade
	{
		m_depthPasses[i].initialize(device, physicalDevice, commandPool, descriptorPool, shadowMapExtents[i], VK_SAMPLE_COUNT_1_BIT, model, glm::mat4(1.0f), false);
	}

	// Render Pass
	m_attachments.resize(2);
	m_attachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
		VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	m_attachments[1].initialize(VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

	m_renderPass.initialize(device, physicalDevice, commandPool, m_attachments, { extent });

	UniformBufferObjectLayout uboLayout;
	uboLayout.accessibility = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayout.binding = 0;

	SamplerLayout samplerLayout;
	samplerLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayout.binding = 1;

	std::vector<ImageLayout> imageLayouts(CASCADE_COUNT);
	for (int i(0); i < CASCADE_COUNT; ++i)
	{
		imageLayouts[i].accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
		imageLayouts[i].binding = 2 + i;
	}

	m_renderer.initialize(device, "Shaders/cascadedShadowMapping/vert.spv", "Shaders/cascadedShadowMapping/frag.spv", { VertexPBR::getBindingDescription(0) }, VertexPBR::getAttributeDescriptions(0),
		{ uboLayout }, {}, imageLayouts, { samplerLayout }, {},  { false });

	m_ubo.initialize(device, physicalDevice, &m_uboData, sizeof(m_uboData));

	m_sampler.initialize(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_NEAREST, 0.0f);

	std::vector<std::pair<Image*, ImageLayout>> images(CASCADE_COUNT);
	for (int i(0); i < CASCADE_COUNT; ++i)
	{
		images[i] = { m_depthPasses[i].getImage(), imageLayouts[i] };
		images[i].first->setImageLayoutWithoutOperation(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
	}

	m_renderer.addMesh(device, descriptorPool, model->getVertexBuffers()[0], { { &m_ubo, uboLayout} }, {}, images, { { &m_sampler, samplerLayout } }, {});

	m_clearValues.resize(2);
	m_clearValues[0] = { 1.0f };
	m_clearValues[1] = { 0.0f, 1.0f, 0.0f, 1.0f };

	m_renderPass.fillCommandBuffer(device, 0, m_clearValues, { &m_renderer }, VK_SAMPLE_COUNT_1_BIT);

	m_outputTexture.createFromImage(device, m_renderPass.getImages(0)[1]);

	float near = cameraNear;
	float far = 32.0f; // we don't render shadows on all the range
	for (float i(1.0f / CASCADE_COUNT); i <= 1.0f; i += 1.0f / CASCADE_COUNT)
	{
		float d_uni = glm::mix(near, far, i);
		float d_log = near * glm::pow((far / near), i);

		m_cascadeSplits.push_back(glm::mix(d_uni, d_log, 0.5f));
	}

	for (int i(0); i < CASCADE_COUNT; ++i)
		m_uboData.cascadeSplits[i] = m_cascadeSplits[i];

	if(m_blurAmount > 0)
		m_blur.initialize(device, physicalDevice, commandPool, descriptorPool, graphicsQueue, &m_outputTexture, m_blurAmount, "Shaders/blur/verticalShadowMask.spv", "Shaders/blur/horizontalShadowMask.spv");
}

void CascadedShadowMapping::submit(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkPhysicalDevice physicalDevice, VkDescriptorPool descriptorPool, glm::mat4 view, glm::mat4 model, glm::mat4 projection, float cameraNear, float cameraFOV, glm::vec3 lightDir,
	glm::vec3 cameraPosition, glm::vec3 cameraOrientation)
{
	if(m_updateBlurAmount >= 0)
	{
		if (m_blurAmount != 0)
			m_blur.cleanup(device, commandPool);

		m_blurAmount = m_updateBlurAmount;
		m_updateBlurAmount = -1;

		if(m_blurAmount > 0)
			m_blur.initialize(device, physicalDevice, commandPool, descriptorPool, graphicsQueue, &m_outputTexture, m_blurAmount, "Shaders/blur/verticalShadowMask.spv", "Shaders/blur/horizontalShadowMask.spv");
	}
	
	updateMatrices(cameraNear, cameraFOV, lightDir, cameraPosition, cameraOrientation, model);

	for (int i(0); i < CASCADE_COUNT; ++i)
		m_depthPasses[i].submit(device, graphicsQueue, m_uboData.lightSpaceMatrices[i]);

	std::vector<Semaphore*> semaphoresToWait(CASCADE_COUNT);
	for (int i(0); i < CASCADE_COUNT; ++i)
		semaphoresToWait[i] = m_depthPasses[i].getSemaphore();

	m_uboData.mvp = projection * view * model;
	m_ubo.updateData(device, &m_uboData);

	m_renderPass.submit(device, graphicsQueue, 0, semaphoresToWait);
	if(m_blurAmount > 0)
		m_blur.submit(device, graphicsQueue, { m_renderPass.getRenderCompleteSemaphore() });
}

void CascadedShadowMapping::cleanup(VkDevice device, VkCommandPool commandPool, VkDescriptorPool descriptorPool)
{
	for (int i(0); i < CASCADE_COUNT; ++i)
		m_depthPasses[i].cleanup(device, commandPool, descriptorPool);
	m_renderPass.cleanup(device, commandPool);
	m_ubo.cleanup(device);
	m_renderer.cleanup(device, descriptorPool);
	m_outputTexture.cleanup(device);
	m_sampler.cleanup(device);
}

void CascadedShadowMapping::setSoftShadowsOption(glm::uint softShadowsOption)
{
	m_uboData.softShadowsOption.x = softShadowsOption;
}

void CascadedShadowMapping::setSSIterations(glm::uint nIterations)
{
	m_uboData.softShadowsOption.y = nIterations;
}

void CascadedShadowMapping::setSamplingDivisor(float divisor)
{
	m_uboData.softShadowsOption.z = static_cast<glm::uint>(divisor);
}

void CascadedShadowMapping::setBlurAmount(int blurAmount)
{
	m_updateBlurAmount = blurAmount;
}

void CascadedShadowMapping::updateMatrices(float cameraNear, float cameraFOV, glm::vec3 lightDir, glm::vec3 cameraPosition, glm::vec3 cameraOrientation, glm::mat4 model)
{
	float lastSplitDist = cameraNear;
	for (int cascade(0); cascade < CASCADE_COUNT; ++cascade)
	{
		const float startCascade = lastSplitDist;
		const float endCascade = m_cascadeSplits[cascade];

		float radius = (endCascade - startCascade) / 2.0f;

		const float ar = static_cast<float>(m_outputTexture.getImage()->getExtent().height) / static_cast<float>(m_outputTexture.getImage()->getExtent().width);
		const float cosHalfHFOV = glm::cos((cameraFOV * (1.0 / ar)) / 2.0f);
		const double b = endCascade / cosHalfHFOV;
		radius = glm::sqrt(b * b + (startCascade + radius) * (startCascade + radius) - 2 * b * startCascade * cosHalfHFOV);

		const float texelPerUnit = static_cast<float>(m_depthPasses[cascade].getImage()->getExtent().width) / (radius * 2.0f);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(texelPerUnit));
		glm::mat4 lookAt = scaleMat * glm::lookAt(glm::vec3(0.0f), -lightDir, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lookAtInv = glm::inverse(lookAt);

		glm::vec3 frustumCenter = cameraPosition + cameraOrientation * startCascade + cameraOrientation * (endCascade / 2.0f);
		frustumCenter = lookAt * glm::vec4(frustumCenter, 1.0f);
		frustumCenter.x = static_cast<float>(floor(frustumCenter.x));
		frustumCenter.y = static_cast<float>(floor(frustumCenter.y));
		frustumCenter = lookAtInv * glm::vec4(frustumCenter, 1.0f);

		glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - glm::normalize(lightDir), frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 proj = glm::ortho(-radius, radius, -radius, radius, -30.0f * 6.0f, 30.0f * 6.0f);
		m_uboData.lightSpaceMatrices[cascade] = proj * lightViewMatrix * model;

		lastSplitDist += m_cascadeSplits[cascade];
	}
}
