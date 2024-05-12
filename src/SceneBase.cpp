#include "SceneBase.h"

#include <string_view>
#include <iostream>
#include "ResourceManager.h"
#include <pugixml.hpp>

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

void SceneBase::Save()
{
	// Create a new XML document
	pugi::xml_document doc;

	// Add a declaration node
	pugi::xml_node decl = doc.append_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "utf-8";

	// Add a root node
	pugi::xml_node modelsNode = doc.append_child("Models");

	for (auto model : m_sceneModels) {
		pugi::xml_node modelNode = modelsNode.append_child("Model");
		modelNode.append_attribute("Name") = model->GetModelName().c_str();
		modelNode.append_attribute("Path") = model->GetModelFullPath().c_str();
		modelNode.append_attribute("World_X") = model->GetPosition().x;
		modelNode.append_attribute("World_Y") = model->GetPosition().y;
		modelNode.append_attribute("World_Z") = model->GetPosition().z;
		modelNode.append_attribute("Scale_X") = model->GetScale().x;
		modelNode.append_attribute("Scale_Y") = model->GetScale().y;
		modelNode.append_attribute("Scale_Z") = model->GetScale().z;
		//modelNode.append_attribute(model->GetModelName().c_str()) = model->GetModelPath().c_str();
	}

	pugi::xml_node staticDirectionalLightsNode = doc.append_child("StaticDirectionalLights");
	for (auto& staticDirectionalLight : m_staticDirectionalLights)
	{
		pugi::xml_node staticDirectionalLightNode = staticDirectionalLightsNode.append_child("StaticDirectionalLight");
		staticDirectionalLightNode.append_attribute("Color_R") = staticDirectionalLight.Color.r;
		staticDirectionalLightNode.append_attribute("Color_G") = staticDirectionalLight.Color.g;
		staticDirectionalLightNode.append_attribute("Color_B") = staticDirectionalLight.Color.b;
		staticDirectionalLightNode.append_attribute("Direction_X") = staticDirectionalLight.Direction.x;
		staticDirectionalLightNode.append_attribute("Direction_Y") = staticDirectionalLight.Direction.y;
		staticDirectionalLightNode.append_attribute("Direction_Z") = staticDirectionalLight.Direction.z;
	}

	pugi::xml_node staticPointLightsNode = doc.append_child("StaticPointLights");
	for (auto& staticPointLight : m_staticPointLights)
	{
		pugi::xml_node staticPointLightNode = staticPointLightsNode.append_child("StaticPointLight");
		staticPointLightNode.append_attribute("Color_R") = staticPointLight.Color.r;
		staticPointLightNode.append_attribute("Color_G") = staticPointLight.Color.g;
		staticPointLightNode.append_attribute("Color_B") = staticPointLight.Color.b;
		staticPointLightNode.append_attribute("Position_X") = staticPointLight.Position.x;
		staticPointLightNode.append_attribute("Position_Y") = staticPointLight.Position.y;
		staticPointLightNode.append_attribute("Position_Z") = staticPointLight.Position.z;
		staticPointLightNode.append_attribute("Rotation_X") = staticPointLight.Rotation.x;
		staticPointLightNode.append_attribute("Rotation_Y") = staticPointLight.Rotation.y;
		staticPointLightNode.append_attribute("Rotation_Z") = staticPointLight.Rotation.z;
	}

	// Save the XML document to a file
	if (!doc.save_file("Example.xml")) {
		std::cerr << "Error saving file" << std::endl;
	}
}

void SceneBase::Load(const pugi::xml_node& sceneNode)
{
	printf("Loading scene..\n");

	// Load all the models
	for (auto models = sceneNode.child("Models"); models; models = models.next_sibling("Models"))
	{
		for (auto model = models.child("Model"); model; model = model.next_sibling("Model"))
		{
			std::string modelName = model.attribute("Name").as_string();
			std::string modelPath = model.attribute("Path").as_string();

			auto modelWorldX = model.attribute("World_X").as_float();
			auto modelWorldY = model.attribute("World_Y").as_float();
			auto modelWorldZ = model.attribute("World_Z").as_float();

			auto modelScaleX = model.attribute("Scale_X").as_float();
			auto modelScaleY = model.attribute("Scale_Y").as_float();
			auto modelScaleZ = model.attribute("Scale_Z").as_float();

			auto loadedModel = ResourceManager::GetInstance().GetModel(modelName, modelPath);
			loadedModel->Scale(glm::vec3(modelScaleX, modelScaleY, modelScaleZ));
			loadedModel->Translate(glm::vec3(modelWorldX, modelWorldY, modelWorldZ));

			AddModel(loadedModel);
			printf("Loaded model - Name: %s, Path: %s \n", model.attribute("Name").as_string(), model.attribute("Path").as_string());
		}
	}

	// Load all the directional lights
	for (auto staticDirectionalLights = sceneNode.child("StaticDirectionalLights"); staticDirectionalLights; staticDirectionalLights = staticDirectionalLights.next_sibling("StaticDirectionalLights"))
	{
		for (auto staticDirectionalLight = staticDirectionalLights.child("StaticDirectionalLight"); staticDirectionalLight; staticDirectionalLight = staticDirectionalLight.next_sibling("StaticDirectionalLight"))
		{
			auto lightColorR = staticDirectionalLight.attribute("Color_R").as_float();
			auto lightColorG = staticDirectionalLight.attribute("Color_G").as_float();
			auto lightColorB = staticDirectionalLight.attribute("Color_B").as_float();

			auto lightDirectionX = staticDirectionalLight.attribute("Direction_X").as_float();
			auto lightDirectionY = staticDirectionalLight.attribute("Direction_Y").as_float();
			auto lightDirectionZ = staticDirectionalLight.attribute("Direction_Z").as_float();

			AddLight(StaticDirectionalLight({ lightColorR, lightColorG, lightColorB }, { lightDirectionX, lightDirectionY, lightDirectionZ }));
		}
	}

	// Load all the point lights
	for (auto staticPointLights = sceneNode.child("StaticPointLights"); staticPointLights; staticPointLights = staticPointLights.next_sibling("StaticPointLights"))
	{
		for (auto staticPointLight = staticPointLights.child("StaticPointLight"); staticPointLight; staticPointLight = staticPointLight.next_sibling("StaticPointLight"))
		{
			auto lightColorR = staticPointLight.attribute("Color_R").as_float();
			auto lightColorG = staticPointLight.attribute("Color_G").as_float();
			auto lightColorB = staticPointLight.attribute("Color_B").as_float();

			auto lightPositionX = staticPointLight.attribute("Position_X").as_float();
			auto lightPositionY = staticPointLight.attribute("Position_Y").as_float();
			auto lightPositionZ = staticPointLight.attribute("Position_Z").as_float();
			
			auto lightRotationX = staticPointLight.attribute("Rotation_X").as_float();
			auto lightRotationY = staticPointLight.attribute("Rotation_Y").as_float();
			auto lightRotationZ = staticPointLight.attribute("Rotation_Z").as_float();

			AddLight(StaticPointLight({ lightColorR, lightColorG, lightColorB }, { lightPositionX, lightPositionY, lightPositionZ }, { lightRotationX, lightRotationY, lightRotationZ }));
		}
	}
}