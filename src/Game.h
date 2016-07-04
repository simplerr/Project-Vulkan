#pragma once
#include "Platform.h"
#include "Timer.h"
#include "Object.h"

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
		
		void InitLowDetailTestCase();
		void InitPipelineTestCase();

		void RenderLoop();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void PrintBenchmark();
	private:
		void InitScene();	// Gets called when the Renderer is created
		bool QueryRenderInitKeys();
		std::string GetPipelineStr();

		Renderer* mRenderer;
		Window* mWindow;
		Camera* mCamera;

		// Wrapper class for the fps counter
		Timer mTimer;

		PipelineEnum mPipeline;

		std::string mTestCaseName;
	};
}
