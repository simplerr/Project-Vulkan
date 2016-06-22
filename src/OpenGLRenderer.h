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
		OpenGLRenderer(Window* window);
		virtual void Cleanup();
		virtual void SetNumThreads();
		virtual void Render();
		virtual void Update();

		virtual void AddModel(StaticModel* model);

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); 

		virtual void SetCamera(Camera* camera);

		GLuint LoadShaders(std::string vertex, std::string fragment);

		void InitOpenGLContext();

		virtual void AddObject(Object* object);

		Model* LoadCachedModel(std::string filename);
	private:
		Camera* mCamera;
		Window* mWindow;
		HDC hdc;
		HGLRC hglrc;

		GLuint program;

		float speedx = 0.02f, speedy = 0.02f;

		// vertex array object
		unsigned int vertexArrayObjID;
		GLuint colorArrayObjID;

		std::vector<OpenGLModel> mModels;
		std::map<std::string, Model*> mModelMap;
	};
}