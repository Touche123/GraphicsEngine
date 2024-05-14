#pragma once

#include <glm/mat4x4.hpp>

#include "Mesh.h"
#include "AABB.h"

#include <memory>
#include <string>
#include <string_view>

struct aiScene;
struct aiNode;
struct aiMesh;

class Model {
public:
	Model() = default;
	Model(const std::string_view Path, const std::string_view Name, const bool flipWindingOrder = false, const bool loadMaterial = true);
	Model(const std::string_view Name, const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const PBRMaterialPtr& material) noexcept;
	Model(const std::string_view Name, const Mesh& mesh) noexcept;
	virtual ~Model() = default;

	void AttachMesh(const Mesh mesh) noexcept;

	// Transformations
	void Scale(const glm::vec3& scale);
	void Rotate(const float radians, const glm::vec3& axis);
	void Translate(const glm::vec3& pos);
	glm::mat4 GetModelMatrix() const;

	// Destroys all OpenGL handles for all submeshes. This should only be called by ResourceManager.
	void Delete();

	auto GetMeshes() const noexcept { return m_meshes; }
	auto GetBoundingBox() const noexcept { return m_aabb; }
	auto GetModelName() const noexcept { return m_name; }
	auto GetModelFolderPath() const noexcept { return m_folderPath; }
	auto GetModelFullPath() const noexcept { return m_fullPath; }
	auto GetPosition() const noexcept { return m_position; }
	auto GetScale() const noexcept { return m_scale; }
	void SetPosition(const glm::vec3& pos);
	void SetSelected(bool selected) { m_selected = selected; }
	bool GetSelected() { return m_selected; }

protected:
	std::vector<Mesh> m_meshes;

private:
	bool loadModel(const std::string_view Path, const bool flipWindingOrder, const bool loadMaterial);
	void processNode(aiNode* node, const aiScene* scene, const bool loadMaterial);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene, const bool loadMaterial);

	// Transformation data
	glm::vec3 m_scale, m_position, m_axis;
	glm::vec3 m_size;

	float m_radians;

	AABB m_aabb; // Model bounding box
	bool m_selected = false;
	// Model name
	const std::string m_name;
	// Location on disk holding model and textures
	std::string m_folderPath;
	std::string m_fullPath;

	std::size_t m_numMats;
};

using ModelPtr = std::shared_ptr<Model>;
