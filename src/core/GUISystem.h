#pragma once

/***********************************************************************************/
// Forward Declarations
struct nk_context;
struct GLFWwindow;
struct FrameStats;
struct RenderSystem;
class SceneBase;
/***********************************************************************************/
class GUISystem {
public:
	GUISystem() noexcept = default;

	GUISystem(GUISystem&&) = delete;
	GUISystem(const GUISystem&) = delete;
	GUISystem& operator=(GUISystem&&) = delete;
	GUISystem& operator=(const GUISystem&) = delete;

	void Init(GLFWwindow* windowPtr);
	void Render(const int framebufferWidth,
		const int framebufferHeight,
		const FrameStats& frameStats, SceneBase* scene);
	void Shutdown() const;
	void Update(RenderSystem* renderSystem, SceneBase* scene);
	void UpdateInput();

private:
	nk_context* m_nuklearContext{ nullptr };
	bool m_guiClicked = false;
};