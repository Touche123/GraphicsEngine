#pragma once

#include "core/model.h"
#include <unordered_map>
#include <optional>
#include <filesystem>

class ResourceManager {
	ResourceManager() = default;
	~ResourceManager() = default;

public:
	static auto& GetInstance()
	{
		static ResourceManager instance;
		return instance;
	}

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

	ModelPtr GetModel(const std::string_view name, const std::string_view path);
	ModelPtr CacheModel(const std::string_view name, const Model model, const bool overwriteIfExists = false);

private:
	std::unordered_map<std::string, ModelPtr> m_modelCache;
};