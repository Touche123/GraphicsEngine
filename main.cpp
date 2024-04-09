#ifdef _DEBUG
// CRT Memory Leak detection
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>
#endif

#include "src/Engine.h"
#include "src/Demos/Demo.h"

int main()
{
#ifdef _DEBUG
    // Detects memory leaks upon program exit
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    Engine engine("Data/config.xml");
    
    const auto scene = std::make_shared<Demo>();
    scene->Init("Demo");

    engine.AddScene(std::static_pointer_cast<SceneBase, Demo>(scene));
    engine.SetActiveScene("Demo");

    engine.Execute();

    return 0;
}