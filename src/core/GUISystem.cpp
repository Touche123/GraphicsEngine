#include "GUISystem.h"

#include "../FrameStats.h"
#include "../ResourceManager.h"

#include <fmt/core.h>
#include <cstddef>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear/nuklear.h>
#include <nuklear/nuklear_glfw_gl4.h>
#include "RenderSystem.h"
#include "../SceneBase.h"

// https://github.com/Immediate-Mode-UI/Nuklear/blob/master/demo/glfw_opengl4/main.c

const constexpr std::size_t MaxVertexBuffer{ 512 * 1024 };
const constexpr std::size_t MaxElementBuffer{ 128 * 1024 };

int SSAOKernelSize = 64;
float SSAOKernelRadius = 0.5f;
float SSAOKernelBias = 0.025f;
int enableTextures = 1;
int EnableHDR = 1;
float HDRExposure = 1.0f;
float ambientStrength = 1.0f;

bool directionalLightRotXChanged = false;
bool directionalLightRotYChanged = false;
bool directionalLightRotZChanged = false;
float directionalLightRotX = 0.0f;
float directionalLightRotY = 0.0f;
float directionalLightRotZ = 0.0f;

float modelPosX = 0.0f;
bool saveScene = false;

/***********************************************************************************/
void GUISystem::Init(GLFWwindow* windowPtr)
{
	m_nuklearContext = nk_glfw3_init(windowPtr, NK_GLFW3_INSTALL_CALLBACKS, MaxVertexBuffer, MaxElementBuffer);

	/* Load Fonts: if none of these are loaded a default font will be used  */
	/* Load Cursor: if you uncomment cursor loading please hide the cursor */
	{
		struct nk_font_atlas* atlas;
		nk_glfw3_font_stash_begin(&atlas);
		nk_glfw3_font_stash_end();
	}
}

/***********************************************************************************/
void GUISystem::Render(const int framebufferWidth,
	const int framebufferHeight, const FrameStats& frameStats)
{
	auto modelPtr = ResourceManager::GetInstance().GetModelByName("Sponza");
	
	struct nk_colorf bg;
	bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

	nk_glfw3_new_frame();

	const auto frameStatFlags = NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NO_INPUT;

	if (nk_begin(m_nuklearContext, "Frame Stats", nk_recti(0, framebufferHeight - 80, 720, 80), frameStatFlags))
	{
		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 1);
		{
			nk_layout_row_push(m_nuklearContext, 720);
			nk_label(
				m_nuklearContext,
				fmt::format("Frame Time: {:.2f} ms ({:.0f} fps) | GPU VRAM Usage: {} MB | RAM Usage: {} MB",
					frameStats.frameTimeMilliseconds,
					1.0 / (frameStats.frameTimeMilliseconds / 1000.0),
					frameStats.videoMemoryUsageKB / 1000,
					frameStats.ramUsageKB / 1000
				).c_str(),
				NK_TEXT_LEFT
			);
		}
		nk_layout_row_end(m_nuklearContext);
	}

	nk_end(m_nuklearContext);

	if (nk_begin(m_nuklearContext, "SSAO", nk_recti(20, 20, 250, 400), NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE))
	{
		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			nk_value_int(m_nuklearContext, "Kernel Size", SSAOKernelSize);
			nk_layout_row_push(m_nuklearContext, 100.0f);
			nk_slider_int(m_nuklearContext, 4, &SSAOKernelSize, 64, 2);
			nk_layout_row_end(m_nuklearContext);
		}
		nk_layout_row_end(m_nuklearContext);

		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			//nk_label(m_nuklearContext, "Kernel Radius:", NK_TEXT_LEFT);
			nk_value_float(m_nuklearContext, "Kernel Radius", SSAOKernelRadius);
			nk_layout_row_push(m_nuklearContext, 100.0f);
			nk_slider_float(m_nuklearContext, 0, &SSAOKernelRadius, 2.0f, 0.001f);
			nk_layout_row_end(m_nuklearContext);
		}
		nk_layout_row_end(m_nuklearContext);

		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			nk_value_float(m_nuklearContext, "Kernel Bias", SSAOKernelBias);
			nk_layout_row_push(m_nuklearContext, 110);
			nk_slider_float(m_nuklearContext, 0.001f, &SSAOKernelBias, 1.0f, 0.001f);
			nk_layout_row_end(m_nuklearContext);
		}
		nk_layout_row_end(m_nuklearContext);

		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			nk_value_float(m_nuklearContext, "Ambient Strength", ambientStrength);
			nk_layout_row_push(m_nuklearContext, 110);
			nk_slider_float(m_nuklearContext, 0.001f, &ambientStrength, 10.0f, 0.001f);
			nk_layout_row_end(m_nuklearContext);
		}
		nk_layout_row_end(m_nuklearContext);

		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			nk_value_float(m_nuklearContext, "Model pos  x", modelPtr->GetPosition().x);
			nk_layout_row_push(m_nuklearContext, 110);
			if (nk_slider_float(m_nuklearContext, -10000.0f, &modelPosX, 10000.0f, 0.001f))
			{
				modelPtr->Translate({ modelPosX, 0, 0 });
			}
			nk_layout_row_end(m_nuklearContext);

		}
		nk_layout_row_end(m_nuklearContext);

		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			if (nk_button_label(m_nuklearContext, "Save Scene"))
			{
				saveScene = true;
			}
			nk_layout_row_end(m_nuklearContext);
		}
		nk_layout_row_end(m_nuklearContext);

		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			nk_value_float(m_nuklearContext, "Directional light rot x", directionalLightRotX);
			nk_layout_row_push(m_nuklearContext, 110);
			if (nk_slider_float(m_nuklearContext, -100.0f, &directionalLightRotX, 100.0f, 0.001f))
			{
				directionalLightRotXChanged = true;
			}
			nk_layout_row_end(m_nuklearContext);
			
		}
		nk_layout_row_end(m_nuklearContext);

		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			nk_value_float(m_nuklearContext, "Directional light rot y", directionalLightRotY);
			nk_layout_row_push(m_nuklearContext, 110);
			if (nk_slider_float(m_nuklearContext, -100.0f, &directionalLightRotY, 100.0f, 0.001f))
			{
				directionalLightRotYChanged = true;
			}
			nk_layout_row_end(m_nuklearContext);

		}
		nk_layout_row_end(m_nuklearContext);

		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			nk_value_float(m_nuklearContext, "Directional light rot z", directionalLightRotZ);
			nk_layout_row_push(m_nuklearContext, 110);
			if (nk_slider_float(m_nuklearContext, -100.0f, &directionalLightRotZ, 100.0f, 0.001f))
			{
				directionalLightRotZChanged = true;
			}
			nk_layout_row_end(m_nuklearContext);

		}
		nk_layout_row_end(m_nuklearContext);

		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			nk_checkbox_label(m_nuklearContext, "Enable textures", &enableTextures);
			nk_layout_row_end(m_nuklearContext);
		}
		nk_layout_row_end(m_nuklearContext);

		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 1);
		{
			nk_layout_row_push(m_nuklearContext, 200);
			nk_checkbox_label(m_nuklearContext, "Enable HDR", &EnableHDR);
			nk_layout_row_end(m_nuklearContext);
		}
		nk_layout_row_begin(m_nuklearContext, NK_STATIC, 0, 2);
		{
			nk_layout_row_push(m_nuklearContext, 220);
			nk_value_float(m_nuklearContext, "Exposure", HDRExposure);
			nk_layout_row_push(m_nuklearContext, 110);
			nk_slider_float(m_nuklearContext, 0.001f, &HDRExposure, 420.0f, 0.001f);
			nk_layout_row_end(m_nuklearContext);
		}
	}
	nk_end(m_nuklearContext);

	nk_glfw3_render(NK_ANTI_ALIASING_ON);
}

/***********************************************************************************/
void GUISystem::Shutdown() const
{
	nk_glfw3_shutdown();
}

void GUISystem::Update(RenderSystem* renderSystem, SceneBase* scene)
{	
	if (renderSystem == nullptr)
		return;

	renderSystem->renderSettings.ssao.KernelSize = SSAOKernelSize;
	renderSystem->renderSettings.ssao.KernelRadius= SSAOKernelRadius;
	renderSystem->renderSettings.ssao.KernelBias = SSAOKernelBias;
	renderSystem->renderSettings.renderPass.EnableTextures = enableTextures;
	renderSystem->renderSettings.postProcessing.hdr.EnableExposure = EnableHDR;
	renderSystem->renderSettings.postProcessing.hdr.Exposure= HDRExposure;
	renderSystem->renderSettings.ambientStrength = ambientStrength;
	renderSystem->DirectionalLightTarget = glm::vec3(directionalLightRotX, directionalLightRotY, directionalLightRotZ);

	/*scene->m_staticDirectionalLights[0].Direction.x = directionalLightRotX;
	scene->m_staticDirectionalLights[0].Direction.y = directionalLightRotY;
	scene->m_staticDirectionalLights[0].Direction.z = directionalLightRotZ;*/
	if (saveScene)
	{
		scene->Save();
		saveScene = false;
	}
		
	if (directionalLightRotXChanged)
	{
		//glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);
		//float angleInRadians = glm::radians(10.0f);
		//

		//// Translate the vector to the origin
		//glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -scene->m_staticDirectionalLights[0].Direction);

		//// Rotate the vector around the origin
		//glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angleInRadians, rotationAxis);

		//// Translate the vector back to its original position
		//glm::mat4 inverseTranslationMatrix = glm::translate(glm::mat4(1.0f), scene->m_staticDirectionalLights[0].Direction);

		//// Combine all transformation matrices
		//glm::mat4 finalTransformationMatrix = inverseTranslationMatrix * rotationMatrix * translationMatrix;

		//glm::vec3 rotatedVector = glm::vec3(finalTransformationMatrix * glm::vec4(scene->m_staticDirectionalLights[0].Direction, 1.0f));

		//
		//scene->m_staticDirectionalLights[0].Direction = glm::vec3(rotatedVector.x, rotatedVector.y, rotatedVector.z);
		//printf("%f\n", scene->m_staticDirectionalLights[0].Direction.x);
		//printf("%f\n", scene->m_staticDirectionalLights[0].Direction.y);
		//printf("%f\n\n", scene->m_staticDirectionalLights[0].Direction.z);
		/*glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);
		float angleInRadians = glm::radians(0.1f);
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), angleInRadians, rotationAxis);



		glm::vec4 rotatedVector = rotationMatrix * glm::vec4(scene->m_staticDirectionalLights[0].Direction, 1.0f);

		scene->m_staticDirectionalLights[0].Direction = glm::vec3(rotatedVector.x, rotatedVector.y, rotatedVector.z);*/
		directionalLightRotXChanged = false;
	}

	if (directionalLightRotYChanged)
	{
		/*glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
		float angleInRadians = glm::radians(0.1f);
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), angleInRadians, rotationAxis);



		glm::vec4 rotatedVector = rotationMatrix * glm::vec4(scene->m_staticDirectionalLights[0].Direction, 1.0f);

		scene->m_staticDirectionalLights[0].Direction = glm::vec3(rotatedVector.x, rotatedVector.y, rotatedVector.z);*/
		directionalLightRotYChanged = false;
	}

	if (directionalLightRotZChanged)
	{
		/*glm::vec3 rotationAxis(0.0f, 0.0f, 1.0f);
		float angleInRadians = glm::radians(0.1f);
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), angleInRadians, rotationAxis);
		glm::vec4 rotatedVector = rotationMatrix * glm::vec4(scene->m_staticDirectionalLights[0].Direction, 1.0f);

		scene->m_staticDirectionalLights[0].Direction = glm::vec3(rotatedVector.x, rotatedVector.y, rotatedVector.z);*/
		directionalLightRotZChanged = false;
	}
	

	/*scene->m_staticDirectionalLights[0].Direction.x = directionalLightRotX;
	scene->m_staticDirectionalLights[0].Direction.y = directionalLightRotY;
	scene->m_staticDirectionalLights[0].Direction.z = directionalLightRotZ;*/
}