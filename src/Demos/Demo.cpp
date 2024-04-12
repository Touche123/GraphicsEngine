#include "Demo.h"

#include "../ResourceManager.h"
#include <fileapi.h>

Demo::Demo()
{

}

void Demo::Init(const std::string_view sceneName)
{
	SceneBase::Init(sceneName);

	pugi::xml_document doc;
	const auto& result{ doc.load_string(ResourceManager::GetInstance().LoadTextFile("Example.xml").data()) };

	Load(doc);
}

void Demo::Update(const double dt)
{
	SceneBase::Update(dt);
}