#pragma once
#include <string>
#include <fstream>
#include "Platform.h"

namespace VulkanLib
{
	class StaticModel;
	class Window;
	class Camera;
	class Object;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();
		
		virtual void Cleanup() = 0;
		virtual void SetupMultithreading(int numThreads) = 0;
		virtual void Render() = 0;
		virtual void Update() = 0;
		virtual void Init() = 0;
		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
		virtual void OutputLog(std::ostream& fout) = 0;

		virtual void AddModel(StaticModel* model) = 0;
		virtual void SetCamera(Camera* camera) = 0;

		virtual void AddObject(Object* object) = 0;

		virtual int GetNumVertices() = 0;
		virtual int GetNumTriangles() = 0;
		virtual int GetNumObjects() = 0;
		virtual std::string GetName() = 0;
		virtual int GetNumThreads() = 0;
		virtual Camera* GetCamera() = 0;
	private:
		
	};
}
