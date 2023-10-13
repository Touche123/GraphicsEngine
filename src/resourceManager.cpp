#include "resourceManager.h"

ModelPtr ResourceManager::GetModel(const std::string_view name, const std::string_view path)
{
	const auto val = m_modelCache.find(path.data());

	if (val == m_modelCache.end())
	{
		return m_modelCache.try_emplace(name.data(), std::make_shared<Model>(path, name)).first->second;
	}

	return val->second;
}

ModelPtr ResourceManager::CacheModel(const std::string_view name, const Model model, const bool overwriteIfExists)
{
	if (overwriteIfExists)
	{
		return m_modelCache.insert_or_assign(name.data(), std::make_shared<Model>(model)).first->second;
	}
	return m_modelCache.try_emplace(name.data(), std::make_shared<Model>(model)).first->second;
}