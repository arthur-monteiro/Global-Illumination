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
			m_text.changeText(&m_vk, text, m_fpsCounterTextID);

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

	createPasses(m_sceneType, m_msaaSamples, true);

	m_swapChainRenderPass.updateImageViewMenuItemOption(&m_vk, m_menu.getOptionImageView());
}

void System::changeGlobalIllumination(std::wstring value)
{
	if (value == L"No")
		m_uboDirLightData.ambient = 0.0f; 
	if (value == L"Ambient Lightning")
		m_uboDirLightData.ambient = 0.2f;

	m_uboDirLight.update(&m_vk, m_uboDirLightData);
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
	m_vk.initialize(1066, 600, "Vulkan Demo", recreateCallback, (void*)this, recreate);

	if (!recreate)
	{
		createRessources();
		m_camera.initialize(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f, 5.0f, m_vk.getSwapChainExtend().width / (float)m_vk.getSwapChainExtend().height);

		m_sceneType = SCENE_TYPE_CASCADED_SHADOWMAP;
	}
	else
		m_camera.setAspect(m_vk.getSwapChainExtend().width / (float)m_vk.getSwapChainExtend().height);

	createPasses(m_sceneType, m_msaaSamples, recreate);
}

void System::createRessources()
{
	m_text.initialize(&m_vk, 48, "Fonts/arial.ttf");
	m_fpsCounterTextID = m_text.addText(&m_vk, L"FPS : 0", glm::vec2(-0.99f, 0.85f), 0.065f, glm::vec3(1.0f));

	m_menu.initialize(&m_vk, "Fonts/arial.ttf", setMenuOptionImageViewCallback, this);
	m_menu.addBooleanItem(&m_vk, L"FPS Counter", drawFPSCounterCallback, true, this, { "", "" });
	int shadowsItemID = m_menu.addPicklistItem(&m_vk, L"Shadows", changeShadowsCallback, this, 1, { L"No", L"Shadow Map", L"Cascaded Shadow Map" });
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
	}
	m_swapChainRenderPass.initialize(&m_vk, { { 0, 0 } }, true, msaaSamples);
	m_offscreenShadowMap.initialize(&m_vk, { { 2048, 2048 } }, false, VK_SAMPLE_COUNT_1_BIT, false, true);
	m_offscreenCascadedShadowMap.initialize(&m_vk, { { 2048, 2048 }, { 1024, 1024 }, { 512, 512 } }, false, VK_SAMPLE_COUNT_1_BIT, false, true);
	if (type == SCENE_TYPE_SHADOWMAP)
		m_vk.setRenderFinishedLastRenderPassSemaphore(m_offscreenShadowMap.getRenderFinishedSemaphore());
	else if (type == SCENE_TYPE_NO_SHADOW)
		m_vk.setRenderFinishedLastRenderPassSemaphore(VK_NULL_HANDLE);
	else if (type == SCENE_TYPE_CASCADED_SHADOWMAP)
		m_vk.setRenderFinishedLastRenderPassSemaphore(m_offscreenCascadedShadowMap.getRenderFinishedSemaphore());

	if (!recreate)
	{
		m_uboModelData.matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
		m_uboModel.load(&m_vk, m_uboModelData, VK_SHADER_STAGE_VERTEX_BIT);

		/* Light + VP final pass */
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
			m_uboDirLightCSMData.usePCF = m_uboDirLightData.usePCF;
			m_uboDirLightCSMData.ambient = m_uboDirLightData.ambient;
			//m_uboDirLightCSMData.cascadeSplits = std::vector<float>(m_cascadeCount);
			m_uboDirLightCSM.load(&m_vk, m_uboDirLightCSMData, VK_SHADER_STAGE_FRAGMENT_BIT);

			m_uboVPData.proj = m_camera.getProjection();
			m_uboVPData.view = m_camera.getViewMatrix();
			m_uboVP.load(&m_vk, m_uboVPData, VK_SHADER_STAGE_VERTEX_BIT);
		}

		/* Cascade Shadow Map */
		{
			m_uboVPCSMData.resize(m_cascadeCount);
			m_uboVPCSM.resize(m_cascadeCount);

			m_cascadeSplits.resize(m_cascadeCount);
			m_cascadeSplits = { 2.0f, 8.0f, 96.0f };
			/*for (uint32_t i = 0; i < m_cascadeCount; i++) {
				float p = (i + 1) / static_cast<float>(m_cascadeCount);
				float log = m_camera.getNear() * std::pow(m_camera.getFar() / m_camera.getNear(), p);
				float uniform = m_camera.getNear() + (m_camera.getFar() - m_camera.getNear()) * p;
				float d = 0.95f * (log - uniform) + uniform;
				m_cascadeSplits[i] = (d - m_camera.getNear()) / (m_camera.getFar() - m_camera.getNear());
			}*/

			for (int i(0); i < m_uboVPCSM.size(); ++i)
			{
				m_uboVPCSM[i].load(&m_vk, {}, VK_SHADER_STAGE_VERTEX_BIT);
			}

			m_uboLightSpaceCSM.load(&m_vk, { }, VK_SHADER_STAGE_VERTEX_BIT);
			m_uboDirLightCSMData.cascadeSplits[0] = 2.0f;
			m_uboDirLightCSMData.cascadeSplits[1] = 8.0f;
			m_uboDirLightCSMData.cascadeSplits[2] = 96.0f;

			updateCSM();

			for (int cascade(0); cascade < m_cascadeCount; ++cascade)
			{
				m_offscreenCascadedShadowMap.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVPCSM[cascade], &m_uboModel } } }, "Shaders/offscreenShadowMap/vert.spv", "", 0, false, cascade);
			}

			m_offscreenCascadedShadowMap.recordDraw(&m_vk);
		}
		/* Shadow Map */
		{
			UniformBufferObjectVP vpTemp;
			vpTemp.proj = glm::ortho(-32.0f, 32.0f, -32.0f, 32.0f, 1.0f, 256.0f);
			//m_uboVPData.proj[1][1] *= -1;
			vpTemp.view = glm::lookAt(glm::vec3(32.0f, 32.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
			m_uboVPShadowMap.load(&m_vk, vpTemp, VK_SHADER_STAGE_VERTEX_BIT);

			m_quad.loadObj(&m_vk, "Models/square.obj", glm::vec3(0.0f, 0.0f, 1.0f));
			m_quad.createTextureSampler(&m_vk, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

			UniformBufferObjectSingleMat uboLightSpaceData;
			uboLightSpaceData.matrix = glm::ortho(-32.0f, 32.0f, -32.0f, 32.0f, 1.0f, 256.0f) * glm::lookAt(glm::vec3(32.0f, 32.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f)) * m_uboModelData.matrix;
			m_uboLightSpace.load(&m_vk, uboLightSpaceData, VK_SHADER_STAGE_VERTEX_BIT);

			m_offscreenShadowMap.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVPShadowMap, &m_uboModel } } }, "Shaders/offscreenShadowMap/vert.spv", "", 0);
		}
	}

	if (type == SCENE_TYPE_SHADOWMAP)
	{
		m_swapChainRenderPass.addMesh(&m_vk, { { { &m_quad }, {}, nullptr, {  m_offscreenShadowMap.getFrameBuffer(0).depthImageView } } },
			"Shaders/renderQuad/vert.spv", "Shaders/renderQuad/frag.spv", 1);
		m_swapChainRenderPass.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVP, &m_uboModel, &m_uboLightSpace, &m_uboDirLight }, nullptr, { m_offscreenShadowMap.getFrameBuffer(1).depthImageView } } },
			"Shaders/pbr_shadowmap_textured/vert.spv", "Shaders/pbr_shadowmap_textured/frag.spv", 6, true);
	}
	else if (type == SCENE_TYPE_CASCADED_SHADOWMAP)
	{
		m_swapChainRenderPass.addMesh(&m_vk, { { { &m_quad }, {}, nullptr, {  m_offscreenCascadedShadowMap.getFrameBuffer(0).depthImageView } } },
			"Shaders/renderQuad/vert.spv", "Shaders/renderQuad/frag.spv", 1);
		m_swapChainRenderPass.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVP, &m_uboModel, &m_uboLightSpaceCSM, &m_uboDirLightCSM }, nullptr,
			{ m_offscreenCascadedShadowMap.getFrameBuffer(0).depthImageView, m_offscreenCascadedShadowMap.getFrameBuffer(1).depthImageView,  m_offscreenCascadedShadowMap.getFrameBuffer(2).depthImageView} } },
			"Shaders/pbr_csm_textured/vert.spv", "Shaders/pbr_csm_textured/frag.spv", 8, true);
	}
	else if (type == SCENE_TYPE_NO_SHADOW)
	{
		m_swapChainRenderPass.addMesh(&m_vk, { { m_sponza.getMeshes(), { &m_uboVP, &m_uboModel, &m_uboDirLight }, nullptr, {  } } },
			"Shaders/pbr_no_shadow_textured/vert.spv", "Shaders/pbr_no_shadow_textured/frag.spv", 5);
	}
	//m_swapChainRenderPass.addMesh(&m_vk, spheres, "Shaders/vertSphere.spv", "Shaders/fragSphere.spv", 0);
	//m_skyboxID = m_swapChainRenderPass.addMesh(&m_vk, { { &m_skybox, { &m_uboVPSkybox } } }, "Shaders/vertSkybox.spv", "Shaders/fragSkybox.spv", 1);

	m_swapChainRenderPass.addMenu(&m_vk, &m_menu);
	m_swapChainRenderPass.addText(&m_vk, &m_text);

	if(!recreate)
		m_offscreenShadowMap.recordDraw(&m_vk);
	m_swapChainRenderPass.recordDraw(&m_vk);
}

void System::updateCSM()
{
	float lastSplitDist = 0.0;
	for (int cascade(0); cascade < m_cascadeCount; ++cascade)
	{
		glm::vec3 frustumCorners[8] = {
			glm::vec3(-1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f, -1.0f,  1.0f),
			glm::vec3(-1.0f, -1.0f,  1.0f)
		};

		// Camera projection
		glm::mat4 invCam = glm::inverse(m_camera.getProjection() * m_camera.getViewMatrix() * m_uboModelData.matrix);
		for (uint32_t i = 0; i < 8; i++) {
			glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
			frustumCorners[i] = glm::vec3(invCorner);
		}

		// Cascade offset
		for (uint32_t i = 0; i < 4; i++) {
			glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
			frustumCorners[i + 4] = frustumCorners[i] + (dist * m_cascadeSplits[cascade]);
			frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
		}

		// Get frustum center
		glm::vec3 frustumCenter = glm::vec3(0.0f);
		for (uint32_t i = 0; i < 8; i++) {
			frustumCenter += frustumCorners[i];
		}
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (uint32_t i = 0; i < 8; i++) {
			float distance = glm::length(frustumCorners[i] - frustumCenter);
			radius = glm::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

		if(cascade == 0)
			frustumCenter = m_camera.getPosition() + m_camera.getOrientation() * (m_cascadeSplits[cascade] / 2.0f);
		else
			frustumCenter = m_camera.getPosition() + m_cascadeSplits[cascade - 1] * m_camera.getOrientation() + m_camera.getOrientation() * (m_cascadeSplits[cascade] / 2.0f);

		UniformBufferObjectVP vpTemp;
		vpTemp.proj = glm::ortho(-10.0f, 10.0f, -m_cascadeSplits[cascade] / 2.0f, cascade == 0 ? m_cascadeSplits[cascade] : m_cascadeSplits[cascade] - m_cascadeSplits[cascade - 1], m_camera.getNear(), m_camera.getFar());
		vpTemp.view = glm::lookAt(frustumCenter - m_lightDir * 50.0f, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

		m_uboVPCSMData[cascade] = vpTemp;
		m_uboVPCSM[cascade].update(&m_vk, m_uboVPCSMData[cascade]);
	}

	glm::mat4 lightSpaceCSM[3];

	for (int i = 0; i < m_cascadeCount; ++i)
	{
		lightSpaceCSM[i] = m_uboVPCSMData[i].proj * m_uboVPCSMData[i].view * m_uboModelData.matrix;
	}

	UniformBufferObjectArrayMat tempMat;
	tempMat.matrices[0] = lightSpaceCSM[0];
	tempMat.matrices[1] = lightSpaceCSM[1];
	tempMat.matrices[2] = lightSpaceCSM[2];

	m_uboLightSpaceCSM.update(&m_vk, tempMat);
}
