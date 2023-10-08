#pragma once
#include <GLFW/glfw3.h>

class Monitor 
{
private:
	GLFWmonitor* currentMonitor;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
	GLFWwindow* window;
	int monitorCount = 0;
	int windowWidth = 1600;
	int windowHeight = 900;
public:
	Monitor();
	GLFWwindow* GetWindow();
	void Init();
	void SwitchMonitor(int monitor);
	int MonitorSizeX, MonitorSizeY;
};

