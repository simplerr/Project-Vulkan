#pragma once
#include "Platform.h"
#include "Timer.h"

namespace VulkanLib
{
	class Renderer;
	class Window;

	/*
		The starting point of the application

		Contains the render loop
	*/
	class Game
	{
	public:
		Game(Renderer* renderer, Window* window);
		Game(Window* window);
		~Game();
		
		void RenderLoop();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		void InitScene();	// Gets called when the Renderer is created

		Renderer* mRenderer;
		Window* mWindow;

		// Wrapper class for the fps counter
		Timer mTimer;
	};
}
