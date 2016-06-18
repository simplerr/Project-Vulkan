#pragma once
#include <GL/glew.h>
#include <gl/glu.h>
#include <string>
#include "Renderer.h"
#include "opengl/loadobj.h"

namespace VulkanLib
{
	class Camera;
	
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

		GLuint LoadShaders(std::string vertex, std::string fragment);

		void InitOpenGLContext();
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

		Model *model;
	};
}