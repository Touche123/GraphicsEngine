#include "GUISystem.h"

#include "../FrameStats.h"


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

	if (nk_begin(m_nuklearContext, "SSAO", nk_recti(20, 20, 150, 400), NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE))
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
			nk_slider_float(m_nuklearContext, 0.001f, &ambientStrength, 1.0f, 0.001f);
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

void GUISystem::Update(RenderSystem* renderSystem)
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

}