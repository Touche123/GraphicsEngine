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

	//auto modelCache = ResourceManager::GetInstance().GetModelCache();

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

	// Save the XML document to a file
	if (!doc.save_file("Example.xml")) {
		std::cerr << "Error saving file" << std::endl;
	}
}

void SceneBase::Load(const pugi::xml_node& sceneNode)
{
	printf("Loading scene..\n");

	for (auto models = sceneNode.child("Models"); models; models = models.next_sibling("Models")) {
		for (auto model = models.child("Model"); model; model = model.next_sibling("Model")) {
			std::string modelName = model.attribute("Name").as_string();
			std::string modelPath = model.attribute("Path").as_string();

			auto modelWorldX = model.attribute("World_X").as_float();
			auto modelWorldY = model.attribute("World_Y").as_float();
			auto modelWorldZ = model.attribute("World_Z").as_float();

			auto modelScaleX = model.attribute("Scale_X").as_float();
			auto modelScaleY = model.attribute("Scale_Y").as_float();
			auto modelScaleZ = model.attribute("Scale_Z").as_float();

			auto loadedModel = ResourceManager::GetInstance().GetModel(modelName, modelPath);
			loadedModel->Translate(glm::vec3(modelWorldX, modelWorldY, modelWorldZ));
			loadedModel->Scale(glm::vec3(modelScaleX, modelScaleY, modelScaleZ));
			
			AddModel(loadedModel);
			printf("Loaded model - Name: %s, Path: %s \n", model.attribute("Name").as_string(), model.attribute("Path").as_string());
		}
	}
}