#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

#include "VulkanApp.h"
#include "Window.h"
#include "Renderer.h"
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"
#include "Game.h"
#include "Camera.h"
#include "Object.h"

using namespace VulkanLib;

// The Vulkan application
//VulkanLib::VulkanApp vulkanApp;

VulkanLib::Game* gGame = nullptr;

#if defined(_WIN32)
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(gGame != nullptr)
		gGame->HandleMessages(hwnd, msg, wParam, lParam);

	// Call default window procedure
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
#elif defined(__linux__)
static void handleEvent(const xcb_generic_event_t *event)
{
	// TODO
}
#endif

#if defined(_WIN32)
// Windows entry point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
#elif defined(__linux__)
// Linux entry point
int main(const int argc, const char *argv[])
#endif
{
	/*
		Create the window
	*/
	VulkanLib::Window window = VulkanLib::Window(1280, 1024);

#if defined(_WIN32)			// Win32
	window.SetupWindow(hInstance, WndProc);
#elif defined(__linux__)	// Linux
	window.SetupWindow();
#endif

	// Add starsphere object
	/*Object* sphere = new Object(glm::vec3(0, 0, 0));
	sphere->SetModel("data/models/sphere.obj");
	sphere->SetScale(glm::vec3(100));
	sphere->SetPipeline(PipelineEnum::STARSPHERE);
	sphere->SetId(OBJECT_ID_SKY);
	renderer->AddObject(sphere);*/

 	/*Object* terrain = new Object(glm::vec3(-1000, 0, -1000));
 	terrain->SetModel("data/textures/fft-terrain.tga");
 	terrain->SetPipeline(PipelineEnum::COLORED);
 	terrain->SetScale(glm::vec3(10, 10, 10));
 	terrain->SetColor(glm::vec3(0.0, 0.9, 0.0));
 	terrain->SetId(OBJECT_ID_TERRAIN);
 	renderer->AddObject(terrain);*/

	// Create the game
	//gGame = new VulkanLib::Game(renderer, &window);
	gGame = new VulkanLib::Game(&window);

	// Game loop
	gGame->RenderLoop();

	delete gGame;
	//delete renderer;

	return 0;
}