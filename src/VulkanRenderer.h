#pragma once
#include "Renderer.h"
#include "VulkanApp.h"
#include <vector>

namespace VulkanLib
{
	class VulkanApp;

	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer(Window* window);

		virtual void Cleanup();
		virtual void SetupMultithreading(int numThreads);
		virtual void Render();
		virtual void Update();

		virtual void AddModel(StaticModel* model);

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual void SetCamera(Camera* camera);

		virtual void AddObject(Object* object);

	private:
		VulkanApp* mVulkanApp;
		Camera* mCamera;
		ModelLoader	mModelLoader;

		std::vector<Object*> mObjects;
		std::map<std::string, StaticModel*> mModelMap;

	};

	

}