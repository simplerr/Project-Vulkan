#pragma once
#include <GL/glew.h>
#include <gl/glu.h>
#include <string>
#include <vector>
#include <map>
#include "Renderer.h"
#include "opengl/loadobj.h"

namespace VulkanLib
{
	class Camera;
	
	struct OpenGLModel
	{
		Object* object;
		Model* mesh;
	};
	
	class OpenGLRenderer : public Renderer
	{
	public:
		OpenGLRenderer(Window* window, bool useInstancing = false);
		virtual void Cleanup();
		virtual void SetupMultithreading(int numThreads);
		virtual void Render();
		virtual void Update();
		virtual void Init();

		virtual void AddModel(StaticModel* model);

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); 
		virtual void OutputLog(std::ostream& fout);

		virtual void SetCamera(Camera* camera);

		GLuint LoadShaders(std::string vertex, std::string fragment);

		void InitOpenGLContext();

		virtual void AddObject(Object* object);

		Model* LoadCachedModel(std::string filename);

		int GetNumVertices();
		int GetNumTriangles();
		int GetNumObjects();
		std::string GetName();
		int GetNumThreads();

		Camera* GetCamera();
	private:
		Camera* mCamera;
		Window* mWindow;
		HDC hdc;
		HGLRC hglrc;

		GLuint program;
		GLuint programBlue;

		float speedx = 0.02f, speedy = 0.02f;

		std::vector<OpenGLModel> mModels;
		std::map<std::string, Model*> mModelMap;

		GLuint mTestTexture;

		int mNumVertices = 0;
		int mNumTriangles = 0;

		bool mUseInstancing;
	};
}