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
		VulkanRenderer(Window* window, bool useIntancing = false);
		VulkanRenderer(Window* window, int numThreads, bool useIntancing = false, bool useStaticCommandBuffers = false);

		virtual void Cleanup();
		virtual void SetupMultithreading(int numThreads);
		virtual void Render();
		virtual void Update();
		virtual void Init();

		virtual void AddModel(StaticModel* model);

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual void SetCamera(Camera* camera);

		virtual void AddObject(Object* object);

		int GetNumVertices();
		int GetNumTriangles();
		int GetNumObjects();
		std::string GetName();
		int GetNumThreads();

		Camera* GetCamera();

	private:
		VulkanApp* mVulkanApp;
		Camera* mCamera;
		ModelLoader	mModelLoader;

		int mNumVertices = 0;
		int mNumTriangles = 0;
		int mNumObjects = 0;
	};

	

}