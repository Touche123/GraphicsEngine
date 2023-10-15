#include "Demo.h"

#include "../ResourceManager.h"

Demo::Demo()
{

}

void Demo::Init(const std::string_view sceneName)
{
	SceneBase::Init(sceneName);

	auto model = ResourceManager::GetInstance().GetModel("Sponza", "Data/Models/crytek-sponza/sponza.obj");
	model->Translate(glm::vec3(0.0f));
	model->Scale(glm::vec3(0.01f));
	AddModel(model);

	//auto model2 = ResourceManager::GetInstance().GetModel("Test3", "Data/Models/test3/test.obj");
	//model2->Translate(glm::vec3(0.0f));
	//model2->Scale(glm::vec3(1.0f));
	//AddModel(model2);

	// Sun
	AddLight(StaticDirectionalLight({ 5.0f, 5.0f, 4.5f }, { 25.0f, 50.0f, 10.0f }));
}