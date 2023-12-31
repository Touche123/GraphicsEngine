#include "RenderSystem.h"

#include "../Graphics/GLShaderProgramFactory.h"
#include "../Camera.h"

#include "../Input.h"
#include "../SceneBase.h"

#include <pugixml.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

#include <iostream>
#include <random>
#include "../ResourceManager.h"
#include "../DebugUtility.h"

// https://developer.download.nvidia.com/opengl/specs/GL_NVX_gpu_memory_info.txt
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049

/***********************************************************************************/
void GLAPIENTRY
MessageCallback( GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar* message,
                const void* userParam )
{
 std::fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
           type, severity, message );
}

/***********************************************************************************/
float ourLerp(float a, float b, float f)
{
	return a + f * (b - a);
}

void RenderSystem::Init(const pugi::xml_node& rendererNode)
{

	m_rendererNode = rendererNode;

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to start GLAD.";
		std::abort();
	}

	queryHardwareCaps();

	std::cout << m_caps << '\n';

#ifndef NDEBUG
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << '\n';
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
	std::cout << "OpenGL Driver Vendor: " << glGetString(GL_VENDOR) << '\n';
	std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << '\n';
#endif

	const auto width = rendererNode.attribute("width").as_uint();
	const auto height = rendererNode.attribute("height").as_uint();

	m_width = width;
	m_height = height;
	
	compileShaders();

	setupScreenquad();
	setupGBuffer();
	setupSSAOBuffer();
	//setupTextureSamplers();

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(MessageCallback, nullptr);
	setDefaultState();
	//glEnable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, width, height);

	static auto& deferredShader = m_shaderCache.at("Deferred");
	static auto& shaderLightingPass = m_shaderCache.at("LightingPass");
	static auto& shaderSSAO = m_shaderCache.at("SSAO");
	static auto& shaderSSAOBlur = m_shaderCache.at("SSAOBlur");
	static auto& shaderHDR = m_shaderCache.at("PostProcess_HDR");

	deferredShader.Bind();
	deferredShader.SetUniformi("gPosition", 0);
	deferredShader.SetUniformi("gNormal", 1);
	deferredShader.SetUniformi("gAlbedoSpec", 2);

	shaderLightingPass.Bind();
	shaderLightingPass.SetUniformi("gPosition", 0);
	shaderLightingPass.SetUniformi("gNormal", 1);
	shaderLightingPass.SetUniformi("gAlbedo", 2);
	shaderLightingPass.SetUniformi("ssao", 3);
	shaderLightingPass.SetUniformi("depthMap", 4);

	shaderSSAO.Bind();
	shaderSSAO.SetUniformi("gPosition", 0);
	shaderSSAO.SetUniformi("gNormal", 1);
	shaderSSAO.SetUniformi("texNoise", 2);

	shaderSSAOBlur.Bind();
	shaderSSAOBlur.SetUniformi("ssaoInput", 0);

	shaderHDR.Bind();
	shaderHDR.SetUniformi("hdrBuffer", 0);

	fboShadows.Init("Shadows");
	fboShadows.Bind();
	glGenTextures(1, &depthCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0 , GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);	

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, fboShadows.GetId());
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf(" Failed to initialize the shadow frame buffer!\n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shaderLightingPass	= ssao.vs			ssao_lighting.fs
	// shaderGeometryPass	= ssao_geometry.vs	ssao_geometry.fs
	// shaderSSAO			= ssao.vs			ssao.fs
	// shaderSSAOBlur		= ssao.vs			ssao_blur.fs
	
}

/***********************************************************************************/
void RenderSystem::Update(const Camera& camera)
{

	// Window size changed.
	if (Input::GetInstance().ShouldResize())
	{
		m_width = Input::GetInstance().GetWidth();
		m_height = Input::GetInstance().GetHeight();

		UpdateView(camera);
		glViewport(0, 0, (GLsizei)m_width, (GLsizei)m_height);
	}

	// Update view matrix inside UBO
	const auto view = camera.GetViewMatrix();
	//glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
	//glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
}

/***********************************************************************************/
void RenderSystem::Shutdown() const
{
	for (const auto& shader : m_shaderCache)
	{
		shader.second.DeleteProgram();
	}
}

/***********************************************************************************/
void RenderSystem::Render(const Camera& camera, RenderListIterator renderListBegin, RenderListIterator renderListEnd, const SceneBase& scene, const bool globalWireframe)
{
	setDefaultState();
	glDepthMask(true);
	glm::mat4 projection = camera.GetProjMatrix((float)m_width, (float)m_height);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);

	// Get the shaders we need (static vars initialized during first render call).
	static auto& gbufferShader = m_shaderCache.at("GBuffer");
	static auto& deferredShader = m_shaderCache.at("Deferred");
	static auto& deferredLightBoxShader = m_shaderCache.at("DeferredLightBox");

	static auto& shaderGeometryPass = m_shaderCache.at("GeometryPass");
	static auto& shaderLightingPass = m_shaderCache.at("LightingPass");
	static auto& shaderSSAO = m_shaderCache.at("SSAO");
	static auto& shaderSSAOBlur = m_shaderCache.at("SSAOBlur");
	static auto& shaderHDR = m_shaderCache.at("PostProcess_HDR");
	static auto& shaderShadows = m_shaderCache.at("Shadows");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 0: Create depth cubemap transformation matrices
	float near_plane = 1.0f;
	float far_plane = 25.0f;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
	/*for (unsigned int i = 0; i < scene.m_staticPointLights.size(); i++)
	{
		glm::vec3 lightPos = scene.m_staticPointLights[i].Position;
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	}*/

	// render scene to depth cubemap
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	fboShadows.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	shaderShadows.Bind();

	for (unsigned int i = 0; i < scene.m_staticPointLights.size(); i++)
	{
		shadowTransforms.clear();
		glm::vec3 lightPos = scene.m_staticPointLights[i].Position;
		shaderShadows.SetUniformf("far_plane", far_plane);
		shaderShadows.SetVec3("lightPos", lightPos);
		
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		for (unsigned int face = 0; face < 6; ++face)
		{
			shaderShadows.SetUniform("shadowData[" + std::to_string(face) + "].mat", shadowTransforms[face]);
		}
		renderModelsNoTextures(shaderShadows, renderListBegin, renderListEnd);
		//renderModelsWithTextures(shaderShadows, renderListBegin, renderListEnd);
	}
	glViewport(0,0, m_width, m_height);

	// 1. geometry pass: render scene's geometry/color data into gbuffer
	// -----------------------------------------------------------------
	fboGBuffer.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderGeometryPass.Bind();
		shaderGeometryPass.SetUniform("projection", projection);
		shaderGeometryPass.SetUniform("view", view);
		shaderGeometryPass.SetUniformi("EnableTextures", renderSettings.renderPass.EnableTextures);
		
		// render models
		renderModelsWithTextures(shaderGeometryPass, renderListBegin, renderListEnd);
		
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 2. generate SSAO texture
	fboSSAO.Bind();
		glClear(GL_COLOR_BUFFER_BIT);
		shaderSSAO.Bind();
		// Send kernel + rotation
		for (unsigned int i = 0; i < 64; i++)
			shaderSSAO.SetVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
		shaderSSAO.SetUniform("projection", projection);
		shaderSSAO.SetUniformi("KernelSize", renderSettings.ssao.KernelSize);
		shaderSSAO.SetUniformf("Radius", renderSettings.ssao.KernelRadius);
		shaderSSAO.SetUniformf("Bias", renderSettings.ssao.KernelBias);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);	
		renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 3. Blur SSAO texture to remove noise
	fboSSAOBlur.Bind();
		glClear(GL_COLOR_BUFFER_BIT);
		shaderSSAOBlur.Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// 4. Lighting pass
	//fbolightingPass.Bind();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

	    shaderLightingPass.Bind();
		shaderLightingPass.SetUniformi("EnableHDR", renderSettings.postProcessing.hdr.EnableExposure);
		shaderLightingPass.SetUniformf("Exposure", renderSettings.postProcessing.hdr.Exposure);
		shaderLightingPass.SetUniformf("far_plane", far_plane);
		shaderLightingPass.SetUniformi("EnableShadows", 1);
		shaderLightingPass.SetVec3("cameraPosition", camera.GetPosition());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedo);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
		

	    for (unsigned int i = 0; i < scene.m_staticPointLights.size(); i++)
	    {
	    	glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(scene.m_staticPointLights[i].Position, 1.0));
	    	shaderLightingPass.SetVec3("lights[" + std::to_string(i) + "].Position", glm::vec3(scene.m_staticPointLights[i].Position));
	    	//shaderLightingPass.SetVec3("lights[" + std::to_string(i) + "].Position", glm::vec3(camera.GetViewMatrix() * glm::vec4(scene.m_staticPointLights[i].Position, 1.0)));
	    	shaderLightingPass.SetVec3("lights[" + std::to_string(i) + "].Color", scene.m_staticPointLights[i].Color);
	    	// update attenuation parameters and calculate radius
	    	const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
	    	const float linear = 0.7f;
	    	const float quadratic = 1.8f;
	    	shaderLightingPass.SetUniformf("lights[" + std::to_string(i) + "].Linear", linear);
	    	shaderLightingPass.SetUniformf("lights[" + std::to_string(i) + "].Quadratic", quadratic);
	    	// then calculate radius of light volume/sphere
	    	const float maxBrightness = std::fmaxf(std::fmaxf(scene.m_staticPointLights[i].Color.r, scene.m_staticPointLights[i].Color.g), scene.m_staticPointLights[i].Color.b);
	    	float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
	    	shaderLightingPass.SetUniformf("lights[" + std::to_string(i) + "].Radius", radius);
	    }
	    shaderLightingPass.SetUniformi("NR_LIGHTS", (int)scene.m_staticPointLights.size());
	    
	    renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);	

	// Apply postprocessing 	
	/*glBindFramebuffer(GL_FRAMEBUFFER, 0);	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderHDR.Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, lightingPassTexture);
		shaderHDR.SetUniformi("EnableHDR", renderSettings.postProcessing.hdr.EnableExposure);
		shaderHDR.SetUniformf("Exposure", renderSettings.postProcessing.hdr.Exposure);
		renderQuad();*/

	 // 2.5. Copy content of geometrys depth buffer to default framebuffer's depth buffer
	 glBindFramebuffer(GL_READ_FRAMEBUFFER, fboGBuffer.GetId());
	 glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	 glBlitFramebuffer(0,0, (GLint)m_width, (GLint)m_height,0,0, (GLint)m_width, (GLint)m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	 glBindFramebuffer(GL_FRAMEBUFFER, 0);

	 deferredLightBoxShader.Bind();
	 deferredLightBoxShader.SetUniform("projection", projection);
	 deferredLightBoxShader.SetUniform("view", view);
	 for (unsigned int i = 0; i < scene.m_staticPointLights.size(); i++)
	 {
	 	model = glm::mat4(1.0f);
		
	 	model = glm::translate(model, scene.m_staticPointLights[i].Position);
	 	model = glm::scale(model, glm::vec3(0.125f));
	 	deferredLightBoxShader.SetUniform("model", model);
	 	deferredLightBoxShader.SetVec3("lightColor", scene.m_staticPointLights[i].Color);
		
	 	DebugUtility::GetInstance().RenderCube();
	 }

	/*glClearColor(0.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static auto& basicShader = m_shaderCache.at("BasicShader");
	basicShader.Bind();
	basicShader.SetUniform("projection", projection);
	basicShader.SetUniform("view", view);
	renderModelsWithTextures(basicShader, renderListBegin, renderListEnd);*/

	return;
}

/***********************************************************************************/
int RenderSystem::GetVideoMemUsageKB() const
{
	GLint currentAvailable{ 0 };

	glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentAvailable);

	return m_caps.TotalVideoMemoryKB - currentAvailable;
}

/***********************************************************************************/
// TODO: This needs to be gutted and put elsewhere
void RenderSystem::UpdateView(const Camera& camera)
{
	m_projMatrix = camera.GetProjMatrix((float)m_width, (float)m_height);
	//glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
	//glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(m_projMatrix));
}

/***********************************************************************************/
void RenderSystem::compileShaders()
{
	m_shaderCache.clear();
	for (auto program = m_rendererNode.child("Program"); program; program = program.next_sibling("Program"))
	{

		// Get all shader files that make up the program
		std::vector<Graphics::ShaderStage> stages;
		for (auto shader = program.child("Shader"); shader; shader = shader.next_sibling("Shader"))
		{
			stages.emplace_back(shader.attribute("path").as_string(), shader.attribute("type").as_string());
		}

		std::string name{ program.attribute("name").as_string() };

		auto shaderProgram{ Graphics::GLShaderProgramFactory::createShaderProgram(name, stages) };

		if (shaderProgram)
		{
			m_shaderCache.try_emplace(name, std::move(shaderProgram.value())); // value_or for default and remove if-check?
		}
	}
}

/***********************************************************************************/
void RenderSystem::queryHardwareCaps()
{
	// Anisotropic filtering
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_caps.MaxAnisotropy);

	// Video memory
	glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &m_caps.TotalVideoMemoryKB);
}

/***********************************************************************************/
void RenderSystem::setDefaultState()
{
	glEnable(GL_DEPTH_TEST);
	
	/*glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);*/
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/***********************************************************************************/
void RenderSystem::renderModelsWithTextures(GLShaderProgram& shader, RenderListIterator renderListBegin, RenderListIterator renderListEnd) const
{
	//glBindSampler(m_samplerPBRTextures, 3);
	//glBindSampler(m_samplerPBRTextures, 4);
	//glBindSampler(m_samplerPBRTextures, 5);
	//glBindSampler(m_samplerPBRTextures, 6);
	// glBindSampler(m_samplerPBRTextures, 7);

	auto begin{ renderListBegin };

	while (begin != renderListEnd)
	{
		shader.SetUniform("model", (*begin)->GetModelMatrix());

		const auto& meshes{ (*begin)->GetMeshes() };
		for (const auto& mesh : meshes)
		{

			//uniform sampler2D texture_diffuse1;
			//uniform sampler2D texture_specular1;
			//glUniform1i(glGetUniformLocation(shader., (name + number).c_str()), i);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh.Material->GetParameterTexture(PBRMaterial::ALBEDO));
			shader.SetUniformi("texture_diffuse1", 0);
			//glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, mesh.Material->GetParameterTexture(PBRMaterial::METALLIC));
			//shader.SetUniformi("texture_specular1", 1);
			//shader.SetUniformi("texture_specular1", mesh.Material->GetParameterTexture(PBRMaterial::METALLIC));
			

		/*	glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, mesh.Material->GetParameterTexture(PBRMaterial::ALBEDO));
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, mesh.Material->GetParameterTexture(PBRMaterial::NORMAL));
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, mesh.Material->GetParameterTexture(PBRMaterial::METALLIC));
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, mesh.Material->GetParameterTexture(PBRMaterial::ROUGHNESS));*/

			//glActiveTexture(GL_TEXTURE7);
			//glBindTexture(GL_TEXTURE_2D, mesh.Material.AOMap);

			mesh.VAO.Bind();
			glDrawElements(GL_TRIANGLES, (GLsizei)mesh.IndexCount, GL_UNSIGNED_INT, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		++begin;
	}

	glBindSampler(m_samplerPBRTextures, 0);
}

/***********************************************************************************/
void RenderSystem::renderModelsNoTextures(GLShaderProgram& shader, RenderListIterator renderListBegin, RenderListIterator renderListEnd) const
{
	auto begin{ renderListBegin };

	while (begin != renderListEnd)
	{
		shader.SetUniform("model", (*begin)->GetModelMatrix());

		const auto& meshes{ (*begin)->GetMeshes() };
		for (const auto& mesh : meshes)
		{
			mesh.VAO.Bind();
			glDrawElements(GL_TRIANGLES, (GLsizei)mesh.IndexCount, GL_UNSIGNED_INT, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		++begin;
	}
}

/***********************************************************************************/
void RenderSystem::renderQuad() const
{
	m_quadVAO.Bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

/***********************************************************************************/
void RenderSystem::renderShadowMap(const SceneBase& scene, RenderListIterator renderListBegin, RenderListIterator renderListEnd)
{
	static auto& shadowDepthShader = m_shaderCache.at("ShadowDepthShader");
	shadowDepthShader.Bind();
	static constexpr float near_plane = 0.0f, far_plane = 100.0f;
	static const glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, 50.0f, -50.0f, near_plane, far_plane);

	static const auto& lightView = glm::lookAt(scene.m_staticDirectionalLights[0].Direction,
		glm::vec3(0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f));

	m_lightSpaceMatrix = lightProjection * lightView;

	shadowDepthShader.SetUniform("lightSpaceMatrix", m_lightSpaceMatrix);

	glCullFace(GL_FRONT); // Solve peter-panning
	glViewport(0, 0, m_shadowMapResolution, m_shadowMapResolution);
	m_shadowFBO.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);

	renderModelsNoTextures(shadowDepthShader, renderListBegin, renderListEnd);

	m_shadowFBO.Unbind();
	glViewport(0, 0, (GLsizei)m_width, (GLsizei)m_height);
	glCullFace(GL_BACK);
}

/***********************************************************************************/
void RenderSystem::setupScreenquad()
{
	const std::array<Vertex, 4> screenQuadVertices {
		// Positions				// GLTexture Coords
		Vertex({ -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f }),
		Vertex({ -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }),
		Vertex({  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f }),
		Vertex({  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f })
	};

	m_quadVAO.Init();
	m_quadVAO.Bind();
	m_quadVAO.AttachBuffer(GLVertexArray::BufferType::ARRAY,
		sizeof(Vertex) * screenQuadVertices.size(),
		GLVertexArray::DrawMode::STATIC,
		screenQuadVertices.data());
	m_quadVAO.EnableAttribute(0, 3, sizeof(Vertex), nullptr);
	m_quadVAO.EnableAttribute(1, 2, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, TexCoords)));
}

/***********************************************************************************/
void RenderSystem::setupTextureSamplers()
{
	// Sampler for PBR textures used on meshes
	glGenSamplers(1, &m_samplerPBRTextures);
	glSamplerParameteri(m_samplerPBRTextures, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(m_samplerPBRTextures, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(m_samplerPBRTextures, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(m_samplerPBRTextures, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameterf(m_samplerPBRTextures, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_caps.MaxAnisotropy);

}

/***********************************************************************************/
void RenderSystem::setupShadowMap()
{
	const static float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	m_shadowFBO.Init("Shadow Depth FBO");
	m_shadowFBO.Bind();

	// Depth texture
	if (m_shadowDepthTexture)
	{
		glDeleteTextures(1, &m_shadowDepthTexture);
	}
	glGenTextures(1, &m_shadowDepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_shadowDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_shadowMapResolution, m_shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Clamp to border to fix over-sampling
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Colour texture for Variance Shadow Mapping (VSM)
	if (m_shadowColorTexture)
	{
		glDeleteTextures(1, &m_shadowColorTexture);
	}
	glGenTextures(1, &m_shadowColorTexture);
	glBindTexture(GL_TEXTURE_2D, m_shadowColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, m_shadowMapResolution, m_shadowMapResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Hardware linear filtering gives us soft shadows for free!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Clamp to border to fix over-sampling
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_caps.MaxAnisotropy); // Anisotropic filtering for sharper angles
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	m_shadowFBO.AttachTexture(m_shadowDepthTexture, GLFramebuffer::AttachmentType::DEPTH);
	m_shadowFBO.AttachTexture(m_shadowColorTexture, GLFramebuffer::AttachmentType::COLOR0);

	m_shadowFBO.Unbind();
}


/***********************************************************************************/
void RenderSystem::setProjectionMatrix(const Camera& camera)
{
	m_projMatrix = camera.GetProjMatrix((float)m_width, (float)m_height);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), value_ptr(m_projMatrix));
}

void RenderSystem::setupGBuffer()
{
	fboGBuffer.Init("GBuffer");
	fboGBuffer.Bind();

	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	// color + specular color buffer
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderSystem::setupSSAOBuffer()
{
	// Create framebuffer to hold SSAO processing stage
	fboSSAO.Init("SSAO");
	fboSSAO.Bind();

	// SSAO color buffer
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;
	// and blur stage
	fboSSAOBlur.Init("SSAO Blur");
	fboSSAOBlur.Bind();
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// generate sample kernel
	// ----------------------
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0f;

		// scale samples s.t. they're more aligned to center of kernel
		scale = ourLerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// generate noise texture
	// ----------------------
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}

	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void RenderSystem::renderDepthPass(GLShaderProgram& shader, RenderListIterator renderListBegin, RenderListIterator renderListEnd) const
{
	auto begin{ renderListBegin };

	while (begin != renderListEnd)
	{
		shader.SetUniform("modelMatrix", (*begin)->GetModelMatrix());

		const auto& meshes{ (*begin)->GetMeshes() };
		for (const auto& mesh : meshes)
		{

			mesh.VAO.Bind();
			glDrawElements(GL_TRIANGLES, (GLsizei)mesh.IndexCount, GL_UNSIGNED_INT, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		++begin;
	}
}

