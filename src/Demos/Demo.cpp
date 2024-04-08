#include "Demo.h"

#include "../ResourceManager.h"
#include <fileapi.h>

Demo::Demo()
{

}

void Demo::Init(const std::string_view sceneName)
{
	SceneBase::Init(sceneName);
	AddLight(StaticDirectionalLight({ 1.0f, 1.0f, 0.95f }, { 20.0f, 70.0f, 20.0f }));
	AddLight(StaticPointLight({ 255, 255, 255 }, { 0, 4, 0 }, { 0, 0, 0 }));

	//auto sponza = ResourceManager::GetInstance().GetModel("Sponza", "Data/Models/crytek-sponza/sponza.obj");
	auto sponza = ResourceManager::GetInstance().GetModel("Sponza", "Data/Models/gltf/sponza2/Sponza.gltf");
	sponza->Translate(glm::vec3(0.0f));
	sponza->Scale(glm::vec3(0.01f));
	AddModel(sponza);
	
	auto defaultCube = ResourceManager::GetInstance().GetModel("DefaultCube", "Data/Models/Primitives/DefaultCube.obj");
	defaultCube->Scale(glm::vec3(1.0f));
	defaultCube->Translate(glm::vec3(2.0f, 1.0f, 0.0f));
	AddModel(defaultCube);

	auto defaultPlane = ResourceManager::GetInstance().GetModel("DefaultPlane", "Data/Models/Primitives/DefaultPlane.obj");
	defaultPlane->Scale(glm::vec3(10.0f));
	defaultPlane->Translate(glm::vec3(0.0f, 0.0f, 0.0f));
	AddModel(defaultPlane);

	// Sun
	//srand(13);
	//for (unsigned int i = 0; i < 1; i++)
	//{
	//	// calculate slightly random offsets
	//	float xPos = static_cast<float>(((rand() % 100) / 100.0) * 7.0 - 2.0);
	//	float yPos = static_cast<float>(((rand() % 100) / 100.0) * 20.0 - 2.0);
	//	float zPos = static_cast<float>(((rand() % 100) / 100.0) * 7.0 - 2.0);
	//	//lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
	//	// also calculate random color
	//	float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
	//	float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
	//	float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
	//	//lightColors.push_back(glm::vec3(rColor, gColor, bColor));
	//	//lightColors.push_back(glm::vec3(1, 1, 1));

	//	AddLight(StaticPointLight({ rColor, gColor, bColor}, { xPos, yPos, zPos }));
	//}
}

void Demo::Update(const double dt)
{
	SceneBase::Update(dt);


}