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

	pugi::xml_document doc;
	const auto& result{ doc.load_string(ResourceManager::GetInstance().LoadTextFile("Example.xml").data()) };

	Load(doc);

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
	
	Save();
}

void Demo::Update(const double dt)
{
	SceneBase::Update(dt);
}