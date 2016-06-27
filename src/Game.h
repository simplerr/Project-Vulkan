#pragma once
#include "Platform.h"
#include "Timer.h"

namespace VulkanLib
{
	class Renderer;
	class Window;
	class Camera;

	/*
		The starting point of the application

		Contains the render loop
	*/
	class Game
	{
	public:
		Game(Window* window);
		~Game();
		
		void RenderLoop();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void PrintBenchmark();
	private:
		void InitScene();	// Gets called when the Renderer is created
		bool QueryRenderInitKeys();

		Renderer* mRenderer;
		Window* mWindow;
		Camera* mCamera;

		// Wrapper class for the fps counter
		Timer mTimer;
	};
}
