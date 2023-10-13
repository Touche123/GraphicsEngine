#pragma once
#include <memory>
#include <string_view>

#include "mesh.h"

class Model {
public:
	Model() = default;
	Model(const std::string_view Path, const std::string_view Name, const bool flipWindingOrder = false, const bool loadMaterial = true);
	Model(const std::string_view Name, const Mesh& mesh) noexcept;
	virtual ~Model() = default;

	auto GetMeshes() const noexcept { return m_meshes; }

protected:
	std::vector<Mesh> m_meshes;

private:
	bool loadModel(const std::string_view Path);
	const std::string m_name;
	std::string m_path;
};

using ModelPtr = std::shared_ptr<Model>;