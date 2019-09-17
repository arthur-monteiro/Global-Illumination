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

		if (m_sceneType == SCENE_TYPE_CASCADED_SHADOWMAP)
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
			//m_text.changeText(&m_vk, text, m_fpsCounterTextID);

			m_fpsCount = 0;
			m_startTimeFPSCounter = currentTime;
		}

		float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

		m_uboDirLightData.camPos = glm::vec4(m_camera.getPosition(), 1.0f);
		m_uboDirLight.update(&m_vk, m_uboDirLightData);

		if(m_sceneType == SCENE_TYPE_SHADOWMAP)
			m_offscreenShadowMap.drawCall(&m_vk);
		else if (m_sceneType == SCENE_TYPE_CASCADED_SHADOWMAP)
		{
			updateCSM();
			m_offscreenCascadedShadowMap.drawCall(&m_vk);
			m_offscreenShadowCalculation.drawCall(&m_vk);
			m_offscreenShadowBlur.drawCall(&m_vk);
		}
		m_swapChainRenderPass.drawCall(&m_vk);
	}

	vkDeviceWaitIdle(m_vk.getDevice());

	return true;
}

void System::cleanup()
{
	std::cout << "Cleanup..." << std::endl;

	//m_skybox.cleanup(m_vk.getDevice());
	//m_wall.cleanup(m_vk.getDevice());
	m_sponza.cleanup(m_vk.getDevice());
	m_quad.cleanup(m_vk.getDevice());

	m_swapChainRenderPass.cleanup(&m_vk);
	m_offscreenShadowMap.cleanup(&m_vk);

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
		m_sceneType = SCENE_TYPE_NO_SHADOW;
	else if (value == L"Shadow Map")
		m_sceneType = SCENE_TYPE_SHADOWMAP;
	else if (value == L"CSM")
		m_sceneType = SCENE_TYPE_CASCADED_SHADOWMAP;

	createPasses(m_sceneType, m_msaaSamples, true);

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
		m_camera.initialize(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f, 5.0f, m_vk.getSwapChainExtend().width / (float)m_vk.getSwapChainExtend().height);

		m_sceneType = SCENE_TYPE_CASCADED_SHADOWMAP;
	}
	else
		m_camera.setAspect(m_vk.getSwapChainExtend().width / (float)m_vk.getSwapChainExtend().height);

	createUniformBufferObjects();
	createPasses(m_sceneType, m_msaaSamples, recreate);
	setSemaphores(); // be sure all passes semaphore are initialized
}

void System::createRessources()
{
	m_text.initialize(&m_vk, 48, "Fonts/arial.ttf");
	m_fpsCounterTextID = m_text.addText(&m_vk, L"FPS : 0", glm::vec2(-0.99f, 0.85f), 0.065f, glm::vec3(1.0f));

	m_menu.initialize(&m_vk, "Fonts/arial.ttf", setMenuOptionImageViewCallback, this);
	m_menu.addBooleanItem(&m_vk, L"FPS Counter", drawFPSCounterCallback, true, this, { "", "" });
	int shadowsItemID = m_menu.addPicklistItem(&m_vk, L"Shadows", changeShadowsCallback, this, 2, { L"No", L"Shadow Map", L"CSM" });
	int pcfItemID = m_menu.addBooleanItem(&m_vk, L"Percentage Closer Filtering", changePCFCallback, true, this, { "Image_options/shadow_no_pcf.JPG", "Image_options/shadow_with_pcf.JPG" });
	m_menu.addPicklistItem(&m_vk, L"MSAA", changeMSAACallback, this, 0, { L"No", L"2x", L"4x", L"8x" });
	m_menu.addPicklistItem(&m_vk, L"Global Illumination", changeGlobalIlluminationCallback, this, 1, { L"No", L"Ambient Lightning" });

	m_menu.addDependency(MENU_ITEM_TYPE_PICKLIST, shadowsItemID, MENU_ITEM_TYPE_BOOLEAN, pcfItemID, { 1 });

	m_sponza.loadObj(&m_vk, "Models/sponza/sponza.obj", "Models/sponza");

	//m_wall.loadObj(&m_vk, "Models/sponza/sponza.obj");
	//m_wall.createTextureSampler(&m_vk, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

void System::createPasses(int type, VkSampleCountFlagBits msaaSamples, bool recreate)
{
	if (recreate)
	{
		m_swapChainRenderPass.cleanup(&m_vk);

		m_uboVPData.proj = m_camera.getProjection(); // change aspect

		//m_swapChainRenderPass.initialize(&m_vk, { { 0, 0 } }, true, msaaSamples);
	}

	if (!recreate ||true) // !!
	{
		m_uboModelData.matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
		m_uboModel.load(&m_vk, m_uboModelData, VK_SHADER_STAGE_VERTEX_BIT);

		/* Light + VP final pass */
		{
            m_swapChainRenderPass.initialize(&m_vk, { { 0, 0 } }, true, msaaSamples);
		}

		/* Cascade Shadow Map */
		if (m_sceneType == SCENE_TYPE_CASCADED_SHADOWMAP)
		{
			// Init passes
			m_offscreenCascadedShadowMap.initialize(&m_vk,
				{ { 8192, 8192 }, { 8192, 8192 }, { 8192, 8192 }, { 8192, 8192 } },
				false, VK_SAMPLE_COUNT_1_BIT, false, true);
			m_offscreenShadowCalculation.initialize(&m_vk, { m_vk.getSwapChainExtend() }, false, VK_SAMPLE_COUNT_1_BIT, true, true, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			VkExtent2D shadowScreenDownscale = { m_vk.getSwapChainExtend().width / 2, m_vk.getSwapChainExtend().height / 2 };

			// Init images
			m_shadowScreenImage.create(&m_vk, shadowScreenDownscale, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

			m_offscreenShadowBlur.initialize(&m_vk, shadowScreenDownscale, { 16, 16, 1 }, "Shaders/pbr_csm_textured/blur/comp.spv", m_shadowScreenImage.getImageView());

			// Add meshes
			PipelineShaders offscreenShadowMap;
			offscreenShadowMap.vertexShader = "Shaders/offscreenShadowMap/vert.spv";
			for (int cascade(0); cascade < m_cascadeCount; ++cascade)
			{
				m_offscreenCascadedShadowMap.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVPCSM[cascade], &m_uboModel } } }, offscreenShadowMap, 0, false, cascade);
			}

			PipelineShaders shadowCalculation;
			shadowCalculation.vertexShader = "Shaders/pbr_csm_textured/shadow_calculation/vert.spv";
			shadowCalculation.fragmentShader = "Shaders/pbr_csm_textured/shadow_calculation/frag.spv";
			m_offscreenShadowCalculation.addMesh(&m_vk,
				{
					{
						m_sponza.getMeshes(), { &m_uboVP, &m_uboModel, &m_uboLightSpaceCSM, &m_uboCascadeSplits },
						nullptr,
						{
							{ m_offscreenCascadedShadowMap.getFrameBuffer(0).depthImageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
							{ m_offscreenCascadedShadowMap.getFrameBuffer(1).depthImageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
							{ m_offscreenCascadedShadowMap.getFrameBuffer(2).depthImageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
							{ m_offscreenCascadedShadowMap.getFrameBuffer(3).depthImageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL }
						}

					}
				},
				shadowCalculation, m_cascadeCount, false
			);		

			// Record draw
			m_offscreenCascadedShadowMap.recordDraw(&m_vk);

			Operation blitOperation;
			blitOperation.type = OPERATION_TYPE_BLIT;
			blitOperation.dstBlitImage = m_shadowScreenImage.getImage();
			blitOperation.dstBlitExtent = shadowScreenDownscale;
			m_offscreenShadowCalculation.recordDraw(&m_vk, { blitOperation });
		}
		/* Shadow Map */
		if (m_sceneType == SCENE_TYPE_SHADOWMAP)
		{
		    // Pass initialization
            m_offscreenShadowMap.initialize(&m_vk, { { 2048, 2048 } }, false, VK_SAMPLE_COUNT_1_BIT, false, true);

            // Add meshes
			m_quad.loadObj(&m_vk, "Models/square.obj", glm::vec3(0.0f, 0.0f, 1.0f));
			m_quad.createTextureSampler(&m_vk, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

			PipelineShaders offscreenShadowMap;
			offscreenShadowMap.vertexShader = "Shaders/offscreenShadowMap/vert.spv";
			m_offscreenShadowMap.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVPShadowMap, &m_uboModel } } }, offscreenShadowMap, 0);

			// Record draw
            m_offscreenShadowMap.recordDraw(&m_vk);
		}
	}

	// Set on screen render pass shaders / meshes
	if (type == SCENE_TYPE_SHADOWMAP)
	{
		PipelineShaders renderQuad;
		renderQuad.vertexShader = "Shaders/renderQuad/vert.spv";
		renderQuad.fragmentShader = "Shaders/renderQuad/frag.spv";
		m_swapChainRenderPass.addMesh(&m_vk, { { { &m_quad }, {}, nullptr, {  { m_offscreenShadowMap.getFrameBuffer(0).depthImageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL } } } },
			renderQuad, 1);

		PipelineShaders pbrShadowMapTextured;
		pbrShadowMapTextured.vertexShader = "Shaders/pbr_shadowmap_textured/vert.spv";
		pbrShadowMapTextured.vertexShader = "Shaders/pbr_shadowmap_textured/frag.spv";
		m_swapChainRenderPass.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVP, &m_uboModel, &m_uboLightSpace, &m_uboDirLight }, nullptr, 
			{ { m_offscreenShadowMap.getFrameBuffer(0).depthImageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL } } } },
			pbrShadowMapTextured, 6, true);
	}
	else if (type == SCENE_TYPE_CASCADED_SHADOWMAP)
    {
		PipelineShaders pbrCsmTextured;
		pbrCsmTextured.vertexShader = "Shaders/pbr_csm_textured/vert.spv";
		pbrCsmTextured.fragmentShader = "Shaders/pbr_csm_textured/frag.spv";
		m_swapChainRenderPass.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVP, &m_uboModel, &m_uboDirLightCSM }, nullptr,
			{ { m_offscreenShadowBlur.getImageView(), VK_IMAGE_LAYOUT_GENERAL } } } },
			pbrCsmTextured, 6, true);
	}
	else if (type == SCENE_TYPE_NO_SHADOW)
	{
		PipelineShaders pbrNoShadowTextured;
		pbrNoShadowTextured.vertexShader = "Shaders/pbr_no_shadow_textured/vert.spv";
		pbrNoShadowTextured.fragmentShader = "Shaders/pbr_no_shadow_textured/frag.spv";
		m_swapChainRenderPass.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVP, &m_uboModel, &m_uboDirLight }, nullptr, { {} } } },
			pbrNoShadowTextured, 5);
	}
	//m_swapChainRenderPass.addMesh(&m_vk, spheres, "Shaders/vertSphere.spv", "Shaders/fragSphere.spv", 0);
	//m_skyboxID = m_swapChainRenderPass.addMesh(&m_vk, { { &m_skybox, { &m_uboVPSkybox } } }, "Shaders/vertSkybox.spv", "Shaders/fragSkybox.spv", 1);

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

		float ar = (float)m_vk.getSwapChainExtend().height / m_vk.getSwapChainExtend().width;
		float tanHalfHFOV = glm::tan((m_camera.getFOV() * (1.0 / ar)) / 2.0f);
		float tanHalfVFOV = glm::tan((m_camera.getFOV()) / 2.0f);

		float xn = startCascade * tanHalfHFOV;
		float xf = endCascade * tanHalfHFOV;
		float yn = startCascade * tanHalfVFOV;
		float yf = endCascade * tanHalfVFOV;

		glm::vec4 frustumCorners[8] = 
		{
			// near face
			glm::vec4(xn, yn, -startCascade, 1.0),
			glm::vec4(-xn, yn, -startCascade, 1.0),
			glm::vec4(xn, -yn, -startCascade, 1.0),
			glm::vec4(-xn, -yn, -startCascade, 1.0),

			// far face
			glm::vec4(xf, yf, -endCascade, 1.0),
			glm::vec4(-xf, yf, -endCascade, 1.0),
			glm::vec4(xf, -yf, -endCascade, 1.0),
			glm::vec4(-xf, -yf, -endCascade, 1.0)
		};

		glm::vec4 frustumCornersL[8];

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::min();

		//glm::vec3 frustumCenter = cascade == 0 ?
		//	/* Cascade == 0 */ m_camera.getPosition() + m_camera.getOrientation() * (m_cascadeSplits[cascade] / 2.0f) :
		//	/* Others */ m_camera.getPosition() + m_cascadeSplits[cascade - 1] * m_camera.getOrientation() + m_camera.getOrientation() * (m_cascadeSplits[cascade] / 2.0f);
		glm::mat4 lightViewMatrix = glm::lookAt(-m_lightDir * 30.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		for (unsigned int j = 0; j < 8; j++)
		{
			// From view to world space
			glm::vec4 vW = glm::inverse(m_camera.getViewMatrix()) * frustumCorners[j];

			// From world to light space
			frustumCornersL[j] = lightViewMatrix * vW;

			minX = std::min(minX, frustumCornersL[j].x);
			maxX = std::max(maxX, frustumCornersL[j].x);
			minY = std::min(minY, frustumCornersL[j].y);
			maxY = std::max(maxY, frustumCornersL[j].y);
			minZ = std::min(minZ, frustumCornersL[j].z);
			maxZ = std::max(maxZ, frustumCornersL[j].z);
		}

		UniformBufferObjectVP vpTemp;
		vpTemp.proj = glm::ortho(minX, maxX, minY, maxY, 0.1f, 100.0f);
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

    if(m_sceneType == SCENE_TYPE_CASCADED_SHADOWMAP)
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

    else if(m_sceneType == SCENE_TYPE_SHADOWMAP)
    {
        UniformBufferObjectVP vpTemp;
        vpTemp.proj = glm::ortho(-32.0f, 32.0f, -32.0f, 32.0f, 1.0f, 256.0f);
        //m_uboVPData.proj[1][1] *= -1;
        vpTemp.view = glm::lookAt(glm::vec3(32.0f, 32.0f, 0.0f),
                                  glm::vec3(0.0f, 0.0f, 0.0f),
                                  glm::vec3(0.0f, 1.0f, 0.0f));
        m_uboVPShadowMap.load(&m_vk, vpTemp, VK_SHADER_STAGE_VERTEX_BIT);

        UniformBufferObjectSingleMat uboLightSpaceData;
        uboLightSpaceData.matrix = glm::ortho(-32.0f, 32.0f, -32.0f, 32.0f, 1.0f, 256.0f) * glm::lookAt(glm::vec3(32.0f, 32.0f, 0.0f),
                                                                                                        glm::vec3(0.0f, 0.0f, 0.0f),
                                                                                                        glm::vec3(0.0f, 1.0f, 0.0f)) * m_uboModelData.matrix;
        m_uboLightSpace.load(&m_vk, uboLightSpaceData, VK_SHADER_STAGE_VERTEX_BIT);
    }
}

void System::setSemaphores()
{
    if (m_sceneType == SCENE_TYPE_SHADOWMAP)
        m_vk.setRenderFinishedLastRenderPassSemaphore(m_offscreenShadowMap.getRenderFinishedSemaphore());
    else if (m_sceneType == SCENE_TYPE_NO_SHADOW)
        m_vk.setRenderFinishedLastRenderPassSemaphore(VK_NULL_HANDLE);
    else if (m_sceneType == SCENE_TYPE_CASCADED_SHADOWMAP)
    {
        m_offscreenShadowCalculation.setSemaphoreToWait(m_vk.getDevice(), {
            { m_offscreenCascadedShadowMap.getRenderFinishedSemaphore(), VK_PIPELINE_STAGE_VERTEX_SHADER_BIT }
        });
		m_offscreenShadowBlur.setSemaphoreToWait(m_vk.getDevice(), {
			{ m_offscreenShadowCalculation.getRenderFinishedSemaphore(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT }
		});
        m_vk.setRenderFinishedLastRenderPassSemaphore(m_offscreenShadowBlur.getRenderFinishedSemaphore());
    }
}
