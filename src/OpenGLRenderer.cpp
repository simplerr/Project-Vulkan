#include "OpenGLRenderer.h"
#include "Window.h"
#include "Camera.h"
#include <GL/glew.h>
#include <gl/glu.h>
#include <glm/glm.hpp>
#include "opengl/GL_utilities.h"
#include <sstream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../external/glm/glm/gtc/matrix_transform.hpp"

#pragma comment(lib, "glu32.lib")

namespace VulkanLib
{
	OpenGLRenderer::OpenGLRenderer(Window* window)
	{
		mWindow = window;

		InitOpenGLContext();

		glewInit();

		// Create the camera
		// Create the camera
		mCamera = new Camera(glm::vec3(-11, -13, 7), 60.0f, (float)mWindow->GetWidth() / (float)mWindow->GetHeight(), 0.1f, 25600.0f);
		mCamera->LookAt(glm::vec3(0, 1, 0));
		mCamera->hack = -1;

		// vertex buffer object, used for uploading the geometry
		unsigned int vertexBufferObjID;
		unsigned int bunnyIndexBufferObjID;
		unsigned int bunnyNormalBufferObjID;
		GLuint colorBufferObjID;

		model = LoadModel("data/models/Crate.obj");

		// GL inits
		//glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
		glClearColor(0.2, 0.2, 0.5, 0);
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		program = LoadShaders("data/shaders/opengl/color.vert", "data/shaders/opengl/color.frag");

		// Upload geometry to the GPU:	
		// Allocate and activate Vertex Array Object
		glGenVertexArrays(1, &vertexArrayObjID);
		glBindVertexArray(vertexArrayObjID);

		// Allocate Vertex Buffer Objects
		glGenBuffers(1, &vertexBufferObjID);	// position
		glGenBuffers(1, &colorBufferObjID);		// color
		glGenBuffers(1, &bunnyIndexBufferObjID);		// color
		glGenBuffers(1, &bunnyNormalBufferObjID);		// color

														// VBO for vertex data
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID);
		glBufferData(GL_ARRAY_BUFFER, model->numVertices * 3 * sizeof(GLfloat), model->vertexArray, GL_STATIC_DRAW);
		glVertexAttribPointer(glGetAttribLocation(program, "InPosL"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(glGetAttribLocation(program, "InPosL"));

		// VBO for normal data
		glBindBuffer(GL_ARRAY_BUFFER, bunnyNormalBufferObjID);
		glBufferData(GL_ARRAY_BUFFER, model->numVertices * 3 * sizeof(GLfloat), model->normalArray, GL_STATIC_DRAW);
		glVertexAttribPointer(glGetAttribLocation(program, "InNormalL"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(glGetAttribLocation(program, "InNormalL"));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunnyIndexBufferObjID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->numIndices*sizeof(GLuint), model->indexArray, GL_STATIC_DRAW);

	}

	void OpenGLRenderer::Cleanup()
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hglrc);

		ReleaseDC(mWindow->GetHwnd(), hdc);

		AnimateWindow(mWindow->GetHwnd(), 200, AW_HIDE | AW_BLEND);

		delete mCamera;
	}

	void OpenGLRenderer::SetNumThreads()
	{

	}

	void OpenGLRenderer::Render()
	{
		// clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 world = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(program, "gWorld"), 1, GL_FALSE, glm::value_ptr(world));						// World
		glUniformMatrix4fv(glGetUniformLocation(program, "gView"), 1, GL_FALSE, glm::value_ptr(mCamera->GetView()));				// View
		glUniformMatrix4fv(glGetUniformLocation(program, "gProjection"), 1, GL_FALSE, glm::value_ptr(mCamera->GetProjection()));	// Projection
		glUniform3fv(glGetUniformLocation(program, "gEyePos"), 1, glm::value_ptr(mCamera->GetPosition())); 						// Eye pos
		glUniform3fv(glGetUniformLocation(program, "gLightDir"), 1, glm::value_ptr(glm::vec3(1, 1, 1)));						// Light dir

		glBindVertexArray(vertexArrayObjID);	// Select VAO

		int size = 100;
		for (int x = 0; x < size; x++)
		{
			for (int z = 0; z < size; z++)
			{
				glUniformMatrix4fv(glGetUniformLocation(program, "gWorld"), 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), glm::vec3(x * 10, 0, z * 10))));						// World

				glDrawElements(GL_TRIANGLES, model->numIndices, GL_UNSIGNED_INT, 0L);
			}
		}

		SwapBuffers(hdc);
	}

	void OpenGLRenderer::Update()
	{
		mCamera->Update();
	}

	void OpenGLRenderer::AddModel(StaticModel* model)
	{

	}

	void OpenGLRenderer::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		mCamera->HandleMessages(hWnd, uMsg, wParam, lParam);
	}

	GLuint OpenGLRenderer::LoadShaders(std::string vertex, std::string fragment)
	{
		// Vertex
		std::ifstream fin(vertex);
		std::stringstream ss;
		ss << fin.rdbuf();
		fin.close();
		std::string shaderText = ss.str();

		const char* shaderSource = shaderText.c_str();
		const GLint length = shaderText.length();

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &shaderSource, NULL);
		glCompileShader(vs);

		// Fragment
		fin.open(fragment);
		std::stringstream ss2;
		ss2 << fin.rdbuf();
		fin.close();
		shaderText = ss2.str();

		const char* shaderSource2 = shaderText.c_str();
		const GLint length2 = shaderText.length();

		GLuint ps = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(ps, 1, &shaderSource2, NULL);
		glCompileShader(ps);

		program = glCreateProgram();
		glAttachShader(program, vs);
		glAttachShader(program, ps);
		glLinkProgram(program);
		glUseProgram(program);

		return program;
	}

	void OpenGLRenderer::InitOpenGLContext()
	{
		// Create HDC
		hdc = GetDC(mWindow->GetHwnd());

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

		if (chosenPixelFormat == 0)
			FatalAppExit(NULL, TEXT("ChoosePixelFormat() failed!"));

		int result = SetPixelFormat(hdc, chosenPixelFormat, &pfd);

		if (result == NULL)
			FatalAppExit(NULL, TEXT("SetPixelFormat() failed!"));

		hglrc = wglCreateContext(hdc);

		wglMakeCurrent(hdc, hglrc);
	}
}