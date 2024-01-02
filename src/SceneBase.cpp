#include "SceneBase.h"

#include <string_view>
#include <iostream>
#include "ResourceManager.h"

/***********************************************************************************/
void SceneBase::Init(const std::string_view sceneName)
{
	m_sceneName = sceneName;
	std::cout << "Loading scene: " << sceneName << std::endl;
}

/***********************************************************************************/
void SceneBase::Update(const double dt)
{
	for (auto& light : m_staticPointLights)
	{
		if (direction)
		{
			light.Position.x += 2 * dt;
		} else
			light.Position.x -= 2 * dt;
		
		if (light.Position.x <= -10 || light.Position.x >= 10)
			direction = !direction;
		
	}
}

/***********************************************************************************/
void SceneBase::AddLight(const StaticDirectionalLight& light)
{
	m_staticDirectionalLights.push_back(light);
}

/***********************************************************************************/
void SceneBase::AddLight(const StaticPointLight& light)
{
	m_staticPointLights.push_back(light);
}

/***********************************************************************************/
void SceneBase::AddLight(const StaticSpotLight& light)
{
	m_staticSpotLights.push_back(light);
}

/***********************************************************************************/
void SceneBase::AddModel(const ModelPtr& model)
{
	m_sceneModels.push_back(model);
}
