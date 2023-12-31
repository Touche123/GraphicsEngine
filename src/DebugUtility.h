#pragma once
#include <glad/glad.h>

class DebugUtility 
{
	DebugUtility() = default;
	~DebugUtility() = default;
private:
	unsigned int cubeVAO = 0;
	unsigned int cubeVBO = 0;

public:
	static auto& GetInstance()
	{
		static DebugUtility instance;
		return instance;
	}

	DebugUtility(const DebugUtility&) = delete;
	DebugUtility& operator=(const DebugUtility&) = delete;

	void RenderCube();
	
};
