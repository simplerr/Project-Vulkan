#include "OpenGLRenderer.h"
#include "Window.h"
#include <GL/glew.h>
#include <gl/glu.h>
#include <glm/glm.hpp>

#pragma comment(lib, "glu32.lib")

namespace VulkanLib
{
	OpenGLRenderer::OpenGLRenderer(Window* window)
	{
		mWindow = window;

		// Create HDC
		hdc = GetDC(window->GetHwnd());

		PIXELFORMATDESCRIPTOR pfd = { 0 };

		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);    // just its size
		pfd.nVersion = 1;   // always 1

		pfd.dwFlags =
			PFD_SUPPORT_OPENGL |			// OpenGL support - not DirectDraw
			PFD_DOUBLEBUFFER |				// double buffering support
			PFD_DRAW_TO_WINDOW;				// draw to the app window, not to a bitmap image

		pfd.iPixelType = PFD_TYPE_RGBA;		// red, green, blue, alpha for each pixel
		pfd.cColorBits = 24;                // 24 bit == 8 bits for red, 8 for green, 8 for blue.
		pfd.cDepthBits = 32;                // 32 bits to measure pixel depth

		int chosenPixelFormat = ChoosePixelFormat(hdc, &pfd);
		// what comes back from ChoosePixelFormat() is
		// an integer identifier for a specific pixel
		// format that Windows ALREADY knows about.
		// If you got 0 back, then there was an error.
		if (chosenPixelFormat == 0)
			FatalAppExit(NULL, TEXT("ChoosePixelFormat() failed!"));

		int result = SetPixelFormat(hdc, chosenPixelFormat, &pfd);

		if (result == NULL)
			FatalAppExit(NULL, TEXT("SetPixelFormat() failed!"));

		hglrc = wglCreateContext(hdc);

		wglMakeCurrent(hdc, hglrc);
	}

	void OpenGLRenderer::Cleanup()
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hglrc);

		ReleaseDC(mWindow->GetHwnd(), hdc);

		AnimateWindow(mWindow->GetHwnd(), 200, AW_HIDE | AW_BLEND);
	}

	void OpenGLRenderer::SetNumThreads()
	{

	}

	void OpenGLRenderer::Render()
	{
		// 1. set up the viewport
		glViewport(0, 0, mWindow->GetWidth(), mWindow->GetHeight()); 
											
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0, (float)mWindow->GetWidth() / (float)mWindow->GetHeight(), 1, 1000);

		// 3. viewing transformation
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		gluLookAt(0, 0, 10,
			0, 0, 0,
			0, 1, 0);

		// 4. modelling transformation and drawing
		glClearColor(0.5, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static float i = 0.01f;
		// Notice that 'i' is a STATIC variable.
		// That's very important. (imagine me saying
		// that like Conchords in "Business Time")
		// http://youtube.com/watch?v=WGOohBytKTU

		// A 'static' variable is created ONCE
		// when the function in which it sits first runs.

		// The static variable will "LIVE ON"
		// between seperate calls to the function
		// in which it lives UNTIL THE PROGRAM ENDS.

		i += 0.0002f;     // increase i by 0.001 from its
						 // it had on the LAST FUNCTION CALL to the draw() function

		float c = glm::cos(i);
		float s = glm::sin(i);

		glBegin(GL_TRIANGLES);
		glColor3f(c, 0, 0);      // red
		glVertex3f(1 + c, 0 + s, 0);

		glColor3f(c, s, 0);      // yellow
		glVertex3f(0 + c, 1 + s, 0);

		glColor3f(s, 0.1f, s);   // magenta
		glVertex3f(-1 + c, 0 + s, 0);
		glEnd();

		//7.  SWAP BUFFERS.
		SwapBuffers(hdc);
	}

	void OpenGLRenderer::Update()
	{

	}

	void OpenGLRenderer::AddModel(StaticModel* model)
	{

	}

	void OpenGLRenderer::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

	}

	void OpenGLRenderer::Display()
	{

	}
}