#include "System.h"

System::System()
{
}

System::~System()
{
	//cleanup();
}

void System::initialize()
{
	create();
}

bool System::mainLoop()
{
	while (!glfwWindowShouldClose(m_vk.getWindow()))
	{
		glfwPollEvents();

		// Event processing
		if (glfwGetKey(m_vk.getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS && m_oldEscapeState == GLFW_RELEASE)
		{
			m_swapChainRenderPass.setDrawMenu(&m_vk, m_swapChainRenderPass.getDrawMenu() ? false : true);
			m_camera.setFixed(m_swapChainRenderPass.getDrawMenu());
		}

		if (m_swapChainRenderPass.getDrawMenu())
			m_menu.update(&m_vk, m_vk.getSwapChainExtend().width, m_vk.getSwapChainExtend().height);

		// Key update
		m_oldEscapeState = glfwGetKey(m_vk.getWindow(), GLFW_KEY_ESCAPE);

		m_camera.update(m_vk.getWindow());

		m_uboVPData.view = m_camera.getViewMatrix();
		m_uboVP.update(&m_vk, m_uboVPData);

		if (m_usedEffects & EFFECT_TYPE_CASCADED_SHADOW_MAPPING)
		{
			m_uboDirLightCSMData.camPos = glm::vec4(m_camera.getPosition(), 1.0f);
			m_uboDirLightCSM.update(&m_vk, m_uboDirLightCSMData);
			updateCSM();
		}

		static auto startTime = std::chrono::steady_clock::now();

		auto currentTime = std::chrono::steady_clock::now();
		m_fpsCount++;
		if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTimeFPSCounter).count() > 1000.0f)
		{
			std::wstring text = L"FPS : " + std::to_wstring(m_fpsCount);
			m_text.changeText(&m_vk, text, m_fpsCounterTextID);

			m_fpsCount = 0;
			m_startTimeFPSCounter = currentTime;

			/*std::cout << m_camera.getPosition().x << " " << m_camera.getPosition().y << " " << m_camera.getPosition().z << std::endl;
			std::cout << m_camera.getTarget().x << " " << m_camera.getTarget().y << " " << m_camera.getTarget().z << std::endl;*/
		}

		float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

		m_uboDirLightData.camPos = glm::vec4(m_camera.getPosition(), 1.0f);
		m_uboDirLight.update(&m_vk, m_uboDirLightData);

		if (m_usedEffects == EFFECT_TYPE_CASCADED_SHADOW_MAPPING) // the only effect activated is CSM
		{
			updateCSM();
			m_offscreenCascadedShadowMap.drawCall(&m_vk);
			m_offscreenShadowCalculation.drawCall(&m_vk);
			for (int i(0); i < m_blurAmount; ++i)
			{
				m_offscreenShadowBlurHorizontal[i].drawCall(&m_vk);
				m_offscreenShadowBlurVertical[i].drawCall(&m_vk);
			}
		}
		m_swapChainRenderPass.drawCall(&m_vk);
	}

	vkDeviceWaitIdle(m_vk.getDevice());

	return true;
}

void System::cleanup()
{
	std::cout << "Cleanup..." << std::endl;

	vkQueueWaitIdle(m_vk.getGraphicalQueue());

	//m_skybox.cleanup(m_vk.getDevice());
	//m_wall.cleanup(m_vk.getDevice());
	m_sponza.cleanup(m_vk.getDevice());
	m_quad.cleanup(m_vk.getDevice());

	m_swapChainRenderPass.cleanup(&m_vk);
	m_offscreenCascadedShadowMap.cleanup(&m_vk);
	m_offscreenShadowCalculation.cleanup(&m_vk);
	for (int i(0); i < m_blurAmount; ++i)
	{
		m_offscreenShadowBlurHorizontal[i].cleanup(m_vk.getDevice());
		m_offscreenShadowBlurVertical[i].cleanup(m_vk.getDevice());
	}

	/*for (int cascade(0); cascade < m_cascadeCount; ++cascade)
	{
		vkFreeMemory(m_vk.getDevice(), m_shadowMapBuffers[cascade].second, nullptr);
		vkDestroyBuffer(m_vk.getDevice(), m_shadowMapBuffers[cascade].first, nullptr);
	}*/

	m_vk.cleanup();
}

void System::setMenuOptionImageView(VkImageView imageView)
{
	m_swapChainRenderPass.updateImageViewMenuItemOption(&m_vk, imageView);
}

void System::changePCF(bool status)
{
	m_uboDirLightData.usePCF = status ? 1.0f : 0.0f;
	m_uboDirLight.update(&m_vk, m_uboDirLightData);

	m_swapChainRenderPass.updateImageViewMenuItemOption(&m_vk, m_menu.getOptionImageView());
}

void System::drawFPSCounter(bool status)
{
	m_swapChainRenderPass.setDrawText(&m_vk, status);

	m_swapChainRenderPass.updateImageViewMenuItemOption(&m_vk, m_menu.getOptionImageView());
}

void System::changeShadows(std::wstring value)
{
	if (value == L"No")
		m_usedEffects = m_usedEffects &~ EFFECT_TYPE_CASCADED_SHADOW_MAPPING;
	else if (value == L"CSM")
		m_usedEffects |= EFFECT_TYPE_CASCADED_SHADOW_MAPPING;

	createPasses(true);
	setSemaphores();

	m_swapChainRenderPass.updateImageViewMenuItemOption(&m_vk, m_menu.getOptionImageView());
}

void System::changeGlobalIllumination(std::wstring value)
{
	if (value == L"No")
	{
		m_uboDirLightData.ambient = 0.0f;
		m_uboDirLightCSMData.ambient = 0.0f;
	}
	if (value == L"Ambient Lightning")
	{
		m_uboDirLightData.ambient = 0.2f;
		m_uboDirLightCSMData.ambient = 0.2f;
	}

	m_uboDirLight.update(&m_vk, m_uboDirLightData);
	m_uboDirLightCSM.update(&m_vk, m_uboDirLightCSMData);
}

void System::changeMSAA(std::wstring value)
{
	if (value == L"No")
		m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	else if(value == L"2x")
		m_msaaSamples = VK_SAMPLE_COUNT_2_BIT;
	else if (value == L"4x")
		m_msaaSamples = VK_SAMPLE_COUNT_4_BIT;
	else if (value == L"8x")
		m_msaaSamples = VK_SAMPLE_COUNT_8_BIT;

	create(true);
}

void System::create(bool recreate)
{
	m_vk.initialize(1280, 720, "Vulkan Demo", recreateCallback, (void*)this, recreate);

	if (!recreate)
	{
		createRessources();
		m_camera.initialize(glm::vec3(1.4f, 1.2f, 0.3f), glm::vec3(2.0f, 0.9f, -0.3f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f, 5.0f, m_vk.getSwapChainExtend().width / (float)m_vk.getSwapChainExtend().height);

		m_usedEffects = EFFECT_TYPE_CASCADED_SHADOW_MAPPING;
	}
	else
		m_camera.setAspect(m_vk.getSwapChainExtend().width / (float)m_vk.getSwapChainExtend().height);

	createUniformBufferObjects();
	createPasses(recreate);
	setSemaphores(); // be sure all passes semaphore are initialized
}

void System::createRessources()
{
	m_text.initialize(&m_vk, 48, "Fonts/arial.ttf");
	m_fpsCounterTextID = m_text.addText(&m_vk, L"FPS : 0", glm::vec2(-0.99f, 0.85f), 0.065f, glm::vec3(1.0f));

	m_menu.initialize(&m_vk, "Fonts/arial.ttf", setMenuOptionImageViewCallback, this);
	m_menu.addBooleanItem(&m_vk, L"FPS Counter", drawFPSCounterCallback, true, this, { "", "" });
	int shadowsItemID = m_menu.addPicklistItem(&m_vk, L"Shadows", changeShadowsCallback, this, 1, { L"No", L"CSM" });
	int pcfItemID = m_menu.addBooleanItem(&m_vk, L"Percentage Closer Filtering", changePCFCallback, true, this, { "Image_options/shadow_no_pcf.JPG", "Image_options/shadow_with_pcf.JPG" });
	m_menu.addPicklistItem(&m_vk, L"MSAA", changeMSAACallback, this, 0, { L"No", L"2x", L"4x", L"8x" });
	m_menu.addPicklistItem(&m_vk, L"Global Illumination", changeGlobalIlluminationCallback, this, 1, { L"No", L"Ambient Lightning" });

	m_menu.addDependency(MENU_ITEM_TYPE_PICKLIST, shadowsItemID, MENU_ITEM_TYPE_BOOLEAN, pcfItemID, { 1 });

	m_sponza.loadObj(&m_vk, "Models/sponza/sponza.obj", "Models/sponza");

	//m_wall.loadObj(&m_vk, "Models/sponza/sponza.obj");
	//m_wall.createTextureSampler(&m_vk, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

void System::createPasses(bool recreate)
{
	if (recreate)
	{
		m_swapChainRenderPass.cleanup(&m_vk);

		m_uboVPData.proj = m_camera.getProjection(); // change aspect

		//m_swapChainRenderPass.initialize(&m_vk, { { 0, 0 } }, true, msaaSamples);
	}

	if (!recreate)
	{
		m_uboModelData.matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)); // reduce sponza size
		m_uboModel.load(&m_vk, m_uboModelData, VK_SHADER_STAGE_VERTEX_BIT);
	}

	{
		/* Light + VP final pass */
		{
			m_swapChainRenderPass.initialize(&m_vk, { m_vk.getSwapChainExtend() }, true, m_msaaSamples, { m_vk.getSwapChainImageFormat() }, m_vk.findDepthFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}

		/* Cascade Shadow Map */
		if (m_usedEffects & EFFECT_TYPE_CASCADED_SHADOW_MAPPING)
		{
			/* Shadow maps pass */
			if (!m_offscreenCascadedShadowMap.getIsInitialized())
			{
				m_offscreenCascadedShadowMap.initialize(&m_vk, m_shadowMapExtents,
					false, VK_SAMPLE_COUNT_1_BIT, {}, { VK_FORMAT_D16_UNORM }, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

				PipelineShaders offscreenShadowMap;
				offscreenShadowMap.vertexShader = "Shaders/pbr_csm_textured/shadowmap/vert.spv";
				//offscreenShadowMap.fragmentShader = "Shaders/pbr_csm_textured/shadowmap/frag.spv";
				for (int cascade(0); cascade < m_cascadeCount; ++cascade)
				{
					m_offscreenCascadedShadowMap.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVPCSM[cascade], &m_uboModel } } }, offscreenShadowMap, 0, false, cascade);
				}

				m_offscreenCascadedShadowMap.recordDraw(&m_vk);
			}
			else
			{
				m_offscreenShadowCalculation.cleanup(&m_vk);
				for (int i(0); i < m_blurAmount; ++i)
				{
					m_offscreenShadowBlurHorizontal[i].cleanup(m_vk.getDevice());
					m_offscreenShadowBlurVertical[i].cleanup(m_vk.getDevice());
				}
			}

			/* Shadow Calculation pass */
			m_offscreenShadowCalculation.initialize(&m_vk, { m_vk.getSwapChainExtend() }, false, VK_SAMPLE_COUNT_1_BIT, { m_vk.getSwapChainImageFormat() }, m_vk.findDepthFormat(),
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			PipelineShaders shadowCalculation;
			shadowCalculation.vertexShader = "Shaders/pbr_csm_textured/shadow_calculation/vert.spv";
			shadowCalculation.fragmentShader = "Shaders/pbr_csm_textured/shadow_calculation/frag.spv";
			m_offscreenShadowCalculation.addMesh(&m_vk,
				{
					{
						m_sponza.getMeshes(), { &m_uboVP, &m_uboModel, &m_uboLightSpaceCSM, &m_uboCascadeSplits },
						nullptr,
						{
							{ m_offscreenCascadedShadowMap.getFrameBuffer(0).depthImage.getImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
							{ m_offscreenCascadedShadowMap.getFrameBuffer(1).depthImage.getImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
							{ m_offscreenCascadedShadowMap.getFrameBuffer(2).depthImage.getImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
							{ m_offscreenCascadedShadowMap.getFrameBuffer(3).depthImage.getImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL }
						}

					}
				},
				shadowCalculation, m_cascadeCount, false
			);

			Operation blitOperation;
			blitOperation.type = OPERATION_TYPE_BLIT;
			VkExtent2D shadowScreenDownscale = { m_vk.getSwapChainExtend().width , m_vk.getSwapChainExtend().height };
			m_shadowScreenImage.create(&m_vk, shadowScreenDownscale, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
				VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
			m_shadowScreenImage.transitionImageLayout(&m_vk, VK_IMAGE_LAYOUT_GENERAL);
			blitOperation.dstImages = { m_shadowScreenImage.getImage() };
			blitOperation.dstBlitExtent = { shadowScreenDownscale };

			m_offscreenShadowCalculation.recordDraw(&m_vk, { blitOperation });

			/* Blur */
			m_offscreenShadowBlurHorizontal.resize(m_blurAmount);
			m_offscreenShadowBlurVertical.resize(m_blurAmount);
			m_offscreenShadowBlurHorizontal[0].initialize(&m_vk, shadowScreenDownscale, { 16, 16, 1 }, "Shaders/pbr_csm_textured/blur/horizontal/comp.spv", m_shadowScreenImage.getImageView());
			m_offscreenShadowBlurVertical[0].initialize(&m_vk, shadowScreenDownscale, { 16, 16, 1 }, "Shaders/pbr_csm_textured/blur/vertical/comp.spv", m_offscreenShadowBlurHorizontal[0].getImageView());
			for (int i(1); i < m_blurAmount; ++i)
			{
				m_offscreenShadowBlurHorizontal[i].initialize(&m_vk, shadowScreenDownscale, { 16, 16, 1 }, "Shaders/pbr_csm_textured/blur/horizontal/comp.spv", 
					m_offscreenShadowBlurVertical[i - 1].getImageView());
				m_offscreenShadowBlurVertical[i].initialize(&m_vk, shadowScreenDownscale, { 16, 16, 1 }, "Shaders/pbr_csm_textured/blur/vertical/comp.spv", 
					m_offscreenShadowBlurHorizontal[i].getImageView());
			}
		}

		/* Reflective Shadow Map */
		if (true || m_usedEffects & EFFECT_TYPE_RSM) // !!
		{
			m_offscreenRSM.initialize(&m_vk, { { 64, 8 } }, false, VK_SAMPLE_COUNT_1_BIT, {
				VK_FORMAT_R32G32B32A32_SFLOAT /* world space*/,
				VK_FORMAT_R32G32B32A32_SFLOAT /* normal */,
				VK_FORMAT_R32G32B32A32_SFLOAT /* flux */ },
				m_vk.findDepthFormat(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}

	// Set on screen render pass shaders / meshes
	if (m_usedEffects == EFFECT_TYPE_CASCADED_SHADOW_MAPPING)
    {
		PipelineShaders pbrCsmTextured;
		pbrCsmTextured.vertexShader = "Shaders/pbr_csm_textured/vert.spv";
		pbrCsmTextured.fragmentShader = "Shaders/pbr_csm_textured/frag.spv";
		m_swapChainRenderPass.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVP, &m_uboModel, &m_uboDirLightCSM }, nullptr,
			{ { m_offscreenShadowBlurVertical[m_blurAmount - 1].getImageView(), VK_IMAGE_LAYOUT_GENERAL } } } },
			pbrCsmTextured, 6, true);
	}
	else if (m_usedEffects == 0)
	{
		PipelineShaders pbrNoShadowTextured;
		pbrNoShadowTextured.vertexShader = "Shaders/pbr_no_shadow_textured/vert.spv";
		pbrNoShadowTextured.fragmentShader = "Shaders/pbr_no_shadow_textured/frag.spv";
		m_swapChainRenderPass.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVP, &m_uboModel, &m_uboDirLight }, nullptr, { {} } } },
			pbrNoShadowTextured, 5);
	}

	m_swapChainRenderPass.addMenu(&m_vk, &m_menu);
	m_swapChainRenderPass.addText(&m_vk, &m_text);

	// Record on screen render pass
	m_swapChainRenderPass.recordDraw(&m_vk);
}

void System::updateCSM()
{
	float lastSplitDist = m_camera.getNear();
	for (int cascade(0); cascade < m_cascadeCount; ++cascade)
	{
		float startCascade = lastSplitDist;
		float endCascade = m_cascadeSplits[cascade];

		//float ar = (float)m_vk.getSwapChainExtend().height / m_vk.getSwapChainExtend().width;
		//float tanHalfHFOV = glm::tan((m_camera.getFOV() * (1.0 / ar)) / 2.0f);
		//float tanHalfVFOV = glm::tan((m_camera.getFOV()) / 2.0f);
		//float cosHalfHFOV = glm::cos((m_camera.getFOV() * (1.0 / ar)) / 2.0f);

		//float xn = startCascade * tanHalfHFOV;
		//float xf = endCascade * tanHalfHFOV;
		//float yn = startCascade * tanHalfVFOV;
		//float yf = endCascade * tanHalfVFOV;

		//glm::vec4 frustumCorners[8] = 
		//{
		//	// near face
		//	glm::vec4(xn, yn, -startCascade * cosHalfHFOV, 1.0),
		//	glm::vec4(-xn, yn, -startCascade * cosHalfHFOV, 1.0),
		//	glm::vec4(xn, -yn, -startCascade * cosHalfHFOV, 1.0),
		//	glm::vec4(-xn, -yn, -startCascade * cosHalfHFOV, 1.0),

		//	// far face
		//	glm::vec4(xf, yf, -endCascade, 1.0),
		//	glm::vec4(-xf, yf, -endCascade, 1.0),
		//	glm::vec4(xf, -yf, -endCascade, 1.0),
		//	glm::vec4(-xf, -yf, -endCascade, 1.0)
		//};

		//glm::vec4 frustumCornersL[8];

		//float minX = std::numeric_limits<float>::max();
		//float maxX = std::numeric_limits<float>::min();
		//float minY = std::numeric_limits<float>::max();
		//float maxY = std::numeric_limits<float>::min();
		//float minZ = std::numeric_limits<float>::max();
		//float maxZ = std::numeric_limits<float>::min();

		////glm::vec3 frustumCenter = cascade == 0 ?
		////	/* Cascade == 0 */ m_camera.getPosition() + m_camera.getOrientation() * (m_cascadeSplits[cascade] / 2.0f) :
		////	/* Others */ m_camera.getPosition() + m_cascadeSplits[cascade - 1] * m_camera.getOrientation() + m_camera.getOrientation() * (m_cascadeSplits[cascade] / 2.0f);
		//glm::mat4 lightViewMatrix = glm::lookAt(-glm::normalize(m_lightDir) * 30.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		//for (unsigned int j = 0; j < 8; j++)
		//{
		//	// From view to world space
		//	glm::vec4 vW = glm::inverse(m_camera.getViewMatrix(/*glm::vec3(m_camera.getOrientation().x, 0.0f, m_camera.getOrientation().z)*/)) * frustumCorners[j];

		//	// From world to light space
		//	frustumCornersL[j] = lightViewMatrix * vW;

		//	minX = std::min(minX, frustumCornersL[j].x);
		//	maxX = std::max(maxX, frustumCornersL[j].x);
		//	minY = std::min(minY, frustumCornersL[j].y);
		//	maxY = std::max(maxY, frustumCornersL[j].y);
		//	minZ = std::min(minZ, frustumCornersL[j].z);
		//	maxZ = std::max(maxZ, frustumCornersL[j].z);
		//}

		//UniformBufferObjectVP vpTemp;
		//vpTemp.proj = glm::ortho(minX, maxX, minY, maxY, 0.1f, 100.0f);
		//vpTemp.view = lightViewMatrix;

		float radius = (endCascade - startCascade) / 2;

		float ar = (float)m_vk.getSwapChainExtend().height / m_vk.getSwapChainExtend().width;
		float cosHalfHFOV = glm::cos((m_camera.getFOV() * (1.0 / ar)) / 2.0f);
		double b = endCascade / cosHalfHFOV;
		radius = glm::sqrt(b * b + (startCascade + radius) * (startCascade + radius) - 2 * b * startCascade * cosHalfHFOV);

		float texelPerUnit = (float)m_shadowMapExtents[cascade].width / (radius * 2.0f);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(texelPerUnit));
		glm::mat4 lookAt = scaleMat * glm::lookAt(glm::vec3(0.0f), -m_lightDir, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lookAtInv = glm::inverse(lookAt);

		glm::vec3 frustrumCenter = m_camera.getPosition() + startCascade * m_camera.getOrientation() + m_camera.getOrientation() * (endCascade / 2.0f);
		frustrumCenter = lookAt * glm::vec4(frustrumCenter, 1.0f);
		frustrumCenter.x = (float)floor(frustrumCenter.x);
		frustrumCenter.y = (float)floor(frustrumCenter.y);
		frustrumCenter = lookAtInv * glm::vec4(frustrumCenter, 1.0f);

		glm::mat4 lightViewMatrix = glm::lookAt(frustrumCenter - glm::normalize(m_lightDir), frustrumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

		UniformBufferObjectVP vpTemp;
		vpTemp.proj = glm::ortho(-radius, radius, -radius, radius, -30.0f * 6.0f, 30.0f * 6.0f);
		vpTemp.view = lightViewMatrix;

		m_uboVPCSMData[cascade] = vpTemp;
		m_uboVPCSM[cascade].update(&m_vk, m_uboVPCSMData[cascade]);

		lastSplitDist += m_cascadeSplits[cascade];
	}

	for (int i = 0; i < m_cascadeCount; ++i)
	{
		m_uboLightSpaceCSMData.matrices[i] = m_uboVPCSMData[i].proj * m_uboVPCSMData[i].view * m_uboModelData.matrix;
	}

	m_uboLightSpaceCSM.update(&m_vk, m_uboLightSpaceCSMData.getData(), m_uboLightSpaceCSMData.getSize());
}

void System::createUniformBufferObjects()
{
    /* In all types */
    {
        m_uboDirLightData.camPos = glm::vec4(m_camera.getPosition(), 1.0f);
        m_uboDirLightData.colorLight = glm::vec4(10.0f);
        m_uboDirLightData.dirLight = glm::vec4(m_lightDir, 0.0f);
        m_uboDirLightData.usePCF = 1.0f;
        m_uboDirLightData.ambient = 0.2f;
        m_uboDirLight.load(&m_vk, m_uboDirLightData, VK_SHADER_STAGE_FRAGMENT_BIT);

        m_uboDirLightCSMData.camPos = m_uboDirLightData.camPos;
        m_uboDirLightCSMData.colorLight = m_uboDirLightData.colorLight;
        m_uboDirLightCSMData.dirLight = m_uboDirLightData.dirLight;
        m_uboDirLightCSMData.ambient = m_uboDirLightData.ambient;
        //m_uboDirLightCSMData.cascadeSplits = std::vector<float>(m_cascadeCount);
        m_uboDirLightCSM.load(&m_vk, m_uboDirLightCSMData, VK_SHADER_STAGE_FRAGMENT_BIT);

        m_uboVPData.proj = m_camera.getProjection();
        m_uboVPData.view = m_camera.getViewMatrix();
        m_uboVP.load(&m_vk, m_uboVPData, VK_SHADER_STAGE_VERTEX_BIT);
    }

    if(m_usedEffects & EFFECT_TYPE_CASCADED_SHADOW_MAPPING)
    {
        m_uboVPCSMData.resize(m_cascadeCount);
        m_uboVPCSM.resize(m_cascadeCount);

        //m_cascadeSplits.resize(m_cascadeCount);
		float near = m_camera.getNear();
		float far = 32.0f; // we don't render shadows on all the range
		for (float i(1.0f / m_cascadeCount); i <= 1.0f; i += 1.0f / m_cascadeCount)
		{
			float d_uni = glm::mix(near, far, i);
			float d_log = near * glm::pow((far / near), i);

			m_cascadeSplits.push_back(glm::mix(d_uni, d_log, 0.5f));
		}
      //  m_cascadeSplits = { 4.0f, 10.0f, 20.0f, 32.0f };

        for (int i(0); i < m_uboVPCSM.size(); ++i)
        {
            m_uboVPCSM[i].load(&m_vk, {}, VK_SHADER_STAGE_VERTEX_BIT);
        }

        m_uboLightSpaceCSMData.matrices = std::vector<glm::mat4>(m_cascadeCount);
        m_uboLightSpaceCSM.load(&m_vk, m_uboLightSpaceCSMData.getData(), m_uboLightSpaceCSMData.getSize(), VK_SHADER_STAGE_VERTEX_BIT);

        m_uboCascadeSplitsData.cascadeSplits.resize(m_cascadeCount);
        for (int i(0); i < m_cascadeCount; ++i)
            m_uboCascadeSplitsData.cascadeSplits[i].x = m_cascadeSplits[i];
        m_uboCascadeSplits.load(&m_vk, m_uboCascadeSplitsData.getData(), m_uboCascadeSplitsData.getSize(), VK_SHADER_STAGE_FRAGMENT_BIT);

        updateCSM();
    }
}

void System::setSemaphores()
{
    if (m_usedEffects == 0)
        m_vk.setRenderFinishedLastRenderPassSemaphore(VK_NULL_HANDLE);
    else if (m_usedEffects == EFFECT_TYPE_CASCADED_SHADOW_MAPPING)
    {
        m_offscreenShadowCalculation.setSemaphoreToWait(m_vk.getDevice(), {
            { m_offscreenCascadedShadowMap.getRenderFinishedSemaphore(), VK_PIPELINE_STAGE_VERTEX_SHADER_BIT }
        });

		m_offscreenShadowBlurHorizontal[0].setSemaphoreToWait(m_vk.getDevice(), {
				{ m_offscreenShadowCalculation.getRenderFinishedSemaphore(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT }
			});
		m_offscreenShadowBlurVertical[0].setSemaphoreToWait(m_vk.getDevice(), {
			{ m_offscreenShadowBlurHorizontal[0].getRenderFinishedSemaphore(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT }
			});
		for (int i(1); i < m_blurAmount; ++i)
		{
			m_offscreenShadowBlurHorizontal[i].setSemaphoreToWait(m_vk.getDevice(), {
				{ m_offscreenShadowBlurVertical[i - 1].getRenderFinishedSemaphore(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT }
			});
			m_offscreenShadowBlurVertical[i].setSemaphoreToWait(m_vk.getDevice(), {
				{ m_offscreenShadowBlurHorizontal[i].getRenderFinishedSemaphore(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT } 
			});
		}
        m_vk.setRenderFinishedLastRenderPassSemaphore(m_offscreenShadowBlurVertical[m_blurAmount - 1].getRenderFinishedSemaphore());
    }
}
