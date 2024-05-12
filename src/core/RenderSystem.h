#pragma once

#include "../Model.h"
#include "../Skybox.h"
#include "../Graphics/GLFrameBuffer.h"
#include "../Graphics/GLVertexArray.h"
#include "../Graphics/GLShaderProgram.h"
#include "../Graphics/HardwareCaps.h"

#include <pugixml.hpp>

#include <unordered_map>
#include <vector>

class Camera;
class SceneBase;
class GLShaderProgram;

struct SSAO {
	int KernelSize;
	float KernelRadius;
	float KernelBias;
};

struct RenderPass{
	bool EnableTextures;
};

struct HDR {
	bool EnableExposure;
	float Exposure;
};
struct PostProcessing {
	HDR hdr;
};
struct RenderSettings {
	SSAO ssao;
	RenderPass renderPass;
	PostProcessing postProcessing;
	float ambientStrength;
};

class RenderSystem {
	using RenderListIterator = std::vector<ModelPtr>::const_iterator;
public:
	void Init(const pugi::xml_node& rendererNode);
	void Update(const Camera& camera);

	void Shutdown() const;

	void UpdateView(const Camera& camera);

	void Render(const Camera& camera,
		RenderListIterator renderListBegin,
		RenderListIterator renderListEnd,
		const SceneBase& scene,
		const bool globalWireFrame = false
	);

	int GetVideoMemUsageKB() const;
	glm::vec3 DirectionalLightTarget;

	RenderSettings renderSettings;

private:
	glm::vec3 ambient;
	float ambientStrength;

	void initBoundingBoxDrawing();
	void compileShaders();
	//
	void queryHardwareCaps();
	// Sets the default state required for rendering
	void setDefaultState();
	// Render models boundingbox contained in the renderlist.
	void renderModelBoundingBox(GLShaderProgram& shader, RenderListIterator renderListBegin, RenderListIterator renderListEnd) const;
	// Render models contained in the renderlist
	void renderModelsWithTextures(GLShaderProgram& shader, RenderListIterator renderListBegin, RenderListIterator renderListEnd) const;
	// Render models without binding textures (for a depth or shadow pass perhaps)
	void renderModelsNoTextures(GLShaderProgram& shader, RenderListIterator renderListBegin, RenderListIterator renderListEnd) const;
	// Render NDC screenquad
	void renderQuad() const;
	// Renders shadowmap
	void renderDirectionalShadowMapping(const SceneBase& scene, RenderListIterator renderListBegin, RenderListIterator renderListEnd);
	// Configure NDC screenquad
	void setupScreenquad();
	// Setup texture samplers
	void setupTextureSamplers();
	// Setup FBO, resolution, etc for shadow mapping
	void setupDirectionalShadowMapping();
	// Sets projection matrix variable and updates UBO
	void setProjectionMatrix(const Camera& camera);
	// Sets Framebuffer for GBuffer
	void setupGBuffer();
	// Sets framebuffer for ssao computation
	void setupSSAOBuffer();

	void renderDepthBuffer(const Camera& camera, RenderListIterator renderListBegin, RenderListIterator renderListEnd);
	void renderDepthPass(GLShaderProgram& shader, RenderListIterator renderListBegin, RenderListIterator renderListEnd) const;

	pugi::xml_node m_rendererNode;

	Graphics::HardwareCaps m_caps;

	// Screen dimensions
	std::size_t m_width{ 0 }, m_height{ 0 };

	// Uniform buffer for projection and view matrix
	GLuint m_uboMatrices{ 0 };

	// Projection matrix
	glm::mat4 m_projMatrix, m_lightSpaceMatrix;

	// Texture samplers
	GLuint m_samplerPBRTextures{ 0 };

	// Shadow mapping
	GLuint m_shadowMapResolution{ 2048 }, m_shadowDepthTexture{ 0 }, m_shadowColorTexture{ 0 };
	GLFramebuffer m_shadowFBO;

	// Depth frame Buffer
	GLFramebuffer m_depthBuffer;

	// Environment map
	Skybox m_skybox;

	// Compiled shader cache
	std::unordered_map<std::string, GLShaderProgram> m_shaderCache;

	// Screen-quad
	GLVertexArray m_quadVAO;

	// Post-Processing
	// HDR
	GLuint m_hdrColorBuffer{ 0 }, m_brightnessThresholdColorBuffer{ 0 };
	GLFramebuffer m_hdrFBO;
	// Bloom
	std::array<GLFramebuffer, 2> m_pingPongFBOs;
	std::array<GLuint, 2> m_pingPongColorBuffers{0, 0};
	// Vibrance
	const float m_vibrance{ 0.1f };
	const glm::vec4 m_coefficient{ 0.299f, 0.587f, 0.114f, 0.0f };

	GLFramebuffer fboGBuffer;
	GLFramebuffer fboSSAO;
	GLFramebuffer fboSSAOBlur;
	GLFramebuffer fbolightingPass;
	GLFramebuffer fboShadows;

	unsigned int lightingPassTexture;
	unsigned int gPosition, gNormal, gAlbedo;
	unsigned int rboDepth;

	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> ssaoNoise;
	unsigned int noiseTexture;
	unsigned int depthCubeMap;
	unsigned int ssaoColorBuffer, ssaoColorBufferBlur;

	// Bounding box
	GLuint boundingBoxVBO;
	GLuint boundingBoxVAO;
	std::vector<glm::vec3> boundingBoxVertices = {
	// Define the vertices of the bounding box
	 // Front face (clockwise)
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, 1.0f, -1.0f),
	glm::vec3(-1.0f, 1.0f, -1.0f),

	// Back face (clockwise)
	glm::vec3(-1.0f, -1.0f, 1.0f),
	glm::vec3(1.0f, -1.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(-1.0f, 1.0f, 1.0f),

	// Top face (clockwise)
	glm::vec3(-1.0f, 1.0f, -1.0f),
	glm::vec3(1.0f, 1.0f, -1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(-1.0f, 1.0f, 1.0f),

	// Bottom face (clockwise)
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, 1.0f),
	glm::vec3(-1.0f, -1.0f, 1.0f),

	// Right face (clockwise)
	glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, -1.0f),

	// Left face (clockwise)
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(-1.0f, -1.0f, 1.0f),
	glm::vec3(-1.0f, 1.0f, 1.0f),
	glm::vec3(-1.0f, 1.0f, -1.0f)
	};

	// lighting info
	// -------------
	std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;

	// Shadow Info
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	std::vector<glm::mat4> shadowTransforms;
};