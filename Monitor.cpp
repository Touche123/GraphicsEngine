#include "Monitor.h"

Monitor::Monitor()
{
	
}

void Monitor::Init()
{
	window = glfwCreateWindow(windowWidth, windowHeight, "LearnOpenGL", NULL, NULL);
	monitors = glfwGetMonitors(&monitorCount);
	SwitchMonitor(1);
}
GLFWwindow* Monitor::GetWindow()
{
	return window;
}

void Monitor::SwitchMonitor(int monitor)
{
	const GLFWvidmode* mode = glfwGetVideoMode(monitors[monitor]);
	int finalx = (mode->width / 2) - (windowWidth / 2);
	int finaly = (mode->height / 2) - (windowHeight / 2);

	glfwSetWindowMonitor(window, NULL, finalx, finaly, windowWidth, windowHeight, GLFW_DONT_CARE);
}

