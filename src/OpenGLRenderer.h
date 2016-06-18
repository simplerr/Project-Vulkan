#pragma once
#include "Renderer.h"

namespace VulkanLib
{
	class OpenGLRenderer : public Renderer
	{
	public:
		virtual void Init(Window* window);
		virtual void SetNumThreads();
		virtual void Render();
		virtual void Update();

		virtual void AddModel(StaticModel* model);

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
	};
}