#include "Window.h"

namespace VulkanLib
{
	Window::Window(int width, int height)
	{
		mWidth = width;
		mHeight = height;
	}

	int Window::GetWidth()
	{
		return mWidth;
	}

	int Window::GetHeight()
	{
		return mHeight;
	}

#if defined(_WIN32)
	// Creates a window that Vulkan can use for rendering
	HWND Window::SetupWindow(HINSTANCE hInstance, WNDPROC WndProc)
	{
		windowInstance = hInstance;

		WNDCLASS wc;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = windowInstance;
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = "VulkanWndClassName";

		if (!RegisterClass(&wc)) {
			MessageBox(0, "RegisterClass FAILED", 0, 0);
			PostQuitMessage(0);
		}

		RECT clientRect;
		clientRect.left = GetSystemMetrics(SM_CXSCREEN) / 2 - mWidth / 2.0f;
		clientRect.right = GetSystemMetrics(SM_CXSCREEN) / 2 + mWidth / 2.0f;
		clientRect.top = GetSystemMetrics(SM_CYSCREEN) / 2 - mHeight / 2.0f;
		clientRect.bottom = GetSystemMetrics(SM_CYSCREEN) / 2 + mHeight / 2.0f;

		DWORD style = true ? WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN : WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_CLIPCHILDREN;

		AdjustWindowRect(&clientRect, style, false);
		int width = clientRect.right - clientRect.left;
		int height = clientRect.bottom - clientRect.top;

		// Create the window with a custom size and make it centered
		// NOTE: WS_CLIPCHILDREN Makes the area under child windows not be displayed. (Useful when rendering DirectX and using windows controls).
		window = CreateWindow("VulkanWndClassName", "Vulkan App",
			style, GetSystemMetrics(SM_CXSCREEN) / 2 - (mWidth / 2),
			GetSystemMetrics(SM_CYSCREEN) / 2 - (mHeight / 2), width, height,
			0, 0, windowInstance, 0);

		if (!window) {
			auto error = GetLastError();
			MessageBox(0, "CreateWindow() failed.", 0, 0);
			PostQuitMessage(0);
		}

		// Show the newly created window.
		ShowWindow(window, SW_SHOW);
		SetForegroundWindow(window);
		SetFocus(window);

		return window;
	}

	HWND Window::GetHwnd()
	{
		return window;
	}

	HINSTANCE Window::GetInstance()
	{
		return windowInstance;
	}

#elif defined(__linux__)
	// Set up a window using XCB and request event types
	xcb_window_t Window::SetupWindow()
	{
		uint32_t value_mask, value_list[32];

		window = xcb_generate_id(connection);

		value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		value_list[0] = screen->black_pixel;
		value_list[1] =
			XCB_EVENT_MASK_KEY_RELEASE |
			XCB_EVENT_MASK_EXPOSURE |
			XCB_EVENT_MASK_STRUCTURE_NOTIFY |
			XCB_EVENT_MASK_POINTER_MOTION |
			XCB_EVENT_MASK_BUTTON_PRESS |
			XCB_EVENT_MASK_BUTTON_RELEASE;

		xcb_create_window(connection,
			XCB_COPY_FROM_PARENT,
			window, screen->root,
			0, 0, width, height, 0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual,
			value_mask, value_list);

		/* Magic code that will send notification when window is destroyed */
		xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 1, 12, "WM_PROTOCOLS");
		xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(connection, cookie, 0);

		xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
		atom_wm_delete_window = xcb_intern_atom_reply(connection, cookie2, 0);

		xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
			window, (*reply).atom, 4, 32, 1,
			&(*atom_wm_delete_window).atom);

		std::string windowTitle = getWindowTitle();
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
			window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
			title.size(), windowTitle.c_str());

		free(reply);

		xcb_map_window(connection, window);

		return(window);
	}

	// Initialize XCB connection
	void Window::InitxcbConnection()
	{
		const xcb_setup_t *setup;
		xcb_screen_iterator_t iter;
		int scr;

		connection = xcb_connect(NULL, &scr);
		if (connection == NULL) {
			printf("Could not find a compatible Vulkan ICD!\n");
			fflush(stdout);
			exit(1);
		}

		setup = xcb_get_setup(connection);
		iter = xcb_setup_roots_iterator(setup);
		while (scr-- > 0)
			xcb_screen_next(&iter);
		screen = iter.data;
	}

	void Window::HandleEvent(const xcb_generic_event_t *event)
	{
		// TODO
	}

	xcb_window_t Window::GetWindow()
	{
		return window;
	}

	xcb_connection_t * Window::GetConnection()
	{
		return connection;
	}

#endif

}	// VulkanLib namespace