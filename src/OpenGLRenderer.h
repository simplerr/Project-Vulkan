#pragma once
#include "Renderer.h"

namespace VulkanLib
{
	class OpenGLRenderer : public Renderer
	{
	public:
		OpenGLRenderer(Window* window);
		virtual void Cleanup();
		virtual void SetNumThreads();
		virtual void Render();
		virtual void Update();

		static void Display();

		virtual void AddModel(StaticModel* model);

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		Window* mWindow;
		HDC hdc;
		HGLRC hglrc;
	};
}