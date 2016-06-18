#pragma once
#include "Platform.h"

namespace VulkanLib
{
	class StaticModel;
	class Window;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();
		
		virtual void Cleanup() = 0;
		virtual void SetNumThreads() = 0;
		virtual void Render() = 0;
		virtual void Update() = 0;
		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

		virtual void AddModel(StaticModel* model) = 0;

	private:
		
	};
}
