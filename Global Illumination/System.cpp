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

		m_uboVPSkyboxData.view = glm::mat4(glm::mat3(m_camera.getViewMatrix()));
		m_uboVPSkybox.update(&m_vk, m_uboVPSkyboxData);

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
		m_swapChainRenderPass.drawCall(&m_vk);
	}

	vkDeviceWaitIdle(m_vk.getDevice());

	return true;
}

void System::cleanup()
{
	std::cout << "Cleanup..." << std::endl;

	//m_skybox.cleanup(m_vk.getDevice());
	m_wall.cleanup(m_vk.getDevice());
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

	createPasses(m_sceneType, true);

	m_swapChainRenderPass.updateImageViewMenuItemOption(&m_vk, m_menu.getOptionImageView());
}

void System::create(bool recreate)
{
	m_vk.initialize(1066, 600, "Vulkan Demo", recreateCallback, (void*)this, recreate);

	if (!recreate)
	{
		createRessources();
		m_camera.initialize(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f, 2.0f);

		m_sceneType = SCENE_TYPE_SHADOWMAP;
	}

	createPasses(m_sceneType, recreate);
}

void System::createRessources()
{
	m_text.initialize(&m_vk, 48, "Fonts/arial.ttf");
	m_fpsCounterTextID = m_text.addText(&m_vk, L"FPS : 0", glm::vec2(-0.99f, 0.85f), 0.065f, glm::vec3(1.0f));

	m_menu.initialize(&m_vk, "Fonts/arial.ttf", setMenuOptionImageViewCallback, this);
	m_menu.addBooleanItem(&m_vk, L"FPS Counter", drawFPSCounterCallback, true, this, { "", "" });
	int shadowsItemID = m_menu.addPicklistItem(&m_vk, L"Shadows", changeShadowsCallback, this, 1, { L"No", L"Shadow Map" });
	int pcfItemID = m_menu.addBooleanItem(&m_vk, L"Percentage Closer Filtering", changePCFCallback, true, this, { "Image_options/shadow_no_pcf.JPG", "Image_options/shadow_with_pcf.JPG" });
	m_menu.addPicklistItem(&m_vk, L"Global Illumination", [](void*, std::wstring) {}, this, 1, { L"No", L"Ambient Lightning" });

	m_menu.addDependency(MENU_ITEM_TYPE_PICKLIST, shadowsItemID, MENU_ITEM_TYPE_BOOLEAN, pcfItemID, { 1 });

	m_wall.loadObj(&m_vk, "Models/wall.obj");
	m_wall.createTextureSampler(&m_vk, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

void System::createPasses(int type, bool recreate)
{
	if (recreate)
	{
		m_swapChainRenderPass.cleanup(&m_vk);
	}
	m_swapChainRenderPass.initialize(&m_vk, false, { 0, 0 }, true, VK_SAMPLE_COUNT_8_BIT);
	if (recreate && type == SCENE_TYPE_SHADOWMAP)
		m_vk.setRenderFinishedLastRenderPassSemaphore(m_offscreenShadowMap.getRenderFinishedSemaphore());
	else if(recreate && type == SCENE_TYPE_NO_SHADOW)
		m_vk.setRenderFinishedLastRenderPassSemaphore(VK_NULL_HANDLE);

	if (!recreate)
	{
		m_offscreenShadowMap.initialize(&m_vk, true, { 2048, 2048 }, false, VK_SAMPLE_COUNT_1_BIT, 1, false, true);

		m_uboVPData.proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 96.0f);
		//m_uboVPData.proj[1][1] *= -1;
		m_uboVPData.view = glm::lookAt(glm::vec3(4.0f, 4.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		m_uboVPShadowMap.load(&m_vk, m_uboVPData, VK_SHADER_STAGE_VERTEX_BIT);

		m_uboVPData.proj = glm::perspective(glm::radians(45.0f), m_vk.getSwapChainExtend().width / (float)m_vk.getSwapChainExtend().height, 0.1f, 96.0f);
		m_uboVPData.proj[1][1] *= -1;
		m_uboVPData.view = m_camera.getViewMatrix();
		m_uboVP.load(&m_vk, m_uboVPData, VK_SHADER_STAGE_VERTEX_BIT);

		m_uboVPSkyboxData.proj = glm::perspective(glm::radians(45.0f), m_vk.getSwapChainExtend().width / (float)m_vk.getSwapChainExtend().height, 0.1f, 10.0f);
		m_uboVPSkyboxData.proj[1][1] *= -1;
		m_uboVPSkyboxData.view = glm::mat4(glm::mat3(m_camera.getViewMatrix()));
		m_uboVPSkybox.load(&m_vk, m_uboVPSkyboxData, VK_SHADER_STAGE_VERTEX_BIT);

		m_uboDirLightData.camPos = glm::vec4(m_camera.getPosition(), 1.0f);
		m_uboDirLightData.colorLight = glm::vec4(10.0f);
		m_uboDirLightData.dirLight = glm::vec4(-1.0f, -1.0f, 0.0f, 0.0f);
		m_uboDirLightData.materialAlbedo = glm::vec4(1.0f);
		m_uboDirLightData.materialMetallic = 0.0f;
		m_uboDirLightData.materialRoughness = 0.8f;
		m_uboDirLightData.usePCF = 1.0f;
		m_uboDirLight.load(&m_vk, m_uboDirLightData, VK_SHADER_STAGE_FRAGMENT_BIT);

		m_quad.loadObj(&m_vk, "Models/square.obj", glm::vec3(0.0f, 0.0f, 1.0f));
		m_quad.createTextureSampler(&m_vk, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

		UniformBufferObjectSingleMat uboLightSpaceData;
		uboLightSpaceData.matrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 96.0f) * glm::lookAt(glm::vec3(4.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		m_uboLightSpace.load(&m_vk, uboLightSpaceData, VK_SHADER_STAGE_VERTEX_BIT);

		m_offscreenShadowMap.addMesh(&m_vk, { { &m_wall, { &m_uboVPShadowMap } } }, "Shaders/offscreenShadowMap/vert.spv", "", 0);
	}

	if (type == SCENE_TYPE_SHADOWMAP)
	{
		m_swapChainRenderPass.addMesh(&m_vk, { { &m_quad, {}, nullptr, {  m_offscreenShadowMap.getFrameBuffer(0).depthImageView } } },
			"Shaders/renderQuad/vert.spv", "Shaders/renderQuad/frag.spv", 1);
		m_swapChainRenderPass.addMesh(&m_vk, { { &m_wall, { &m_uboVP, &m_uboLightSpace, &m_uboDirLight }, nullptr, { m_offscreenShadowMap.getFrameBuffer(0).depthImageView } } },
			"Shaders/vert.spv", "Shaders/frag.spv", 1);
	}
	else if (type == SCENE_TYPE_NO_SHADOW)
	{
		m_swapChainRenderPass.addMesh(&m_vk, { { &m_wall, { &m_uboVP, &m_uboLightSpace, &m_uboDirLight }, nullptr, {  } } },
			"Shaders/pbr_no_shadow/vert.spv", "Shaders/pbr_no_shadow/frag.spv", 0);
	}
	//m_swapChainRenderPass.addMesh(&m_vk, spheres, "Shaders/vertSphere.spv", "Shaders/fragSphere.spv", 0);
	//m_skyboxID = m_swapChainRenderPass.addMesh(&m_vk, { { &m_skybox, { &m_uboVPSkybox } } }, "Shaders/vertSkybox.spv", "Shaders/fragSkybox.spv", 1);

	m_swapChainRenderPass.addMenu(&m_vk, &m_menu);
	m_swapChainRenderPass.addText(&m_vk, &m_text);

	if(!recreate)
		m_offscreenShadowMap.recordDraw(&m_vk);
	m_swapChainRenderPass.recordDraw(&m_vk);
}