#include "Window.h"
#include "../Event/ApplicationEvent.h"
#include "../Event/KeyEvent.h"
#include "../Event/MouseEvent.h"

#ifdef GDX11_IMGUI_SUPPORT
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // GDX11_IMGUI_SUPPORT


namespace GDX11
{
	Window::WindowClass::WindowClass(const std::string& windowClassName)
		: m_className(windowClassName), m_hInst(GetModuleHandleA(nullptr))
	{
		WNDCLASSEXA wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXA);
		wcex.style = CS_OWNDC;
		wcex.lpfnWndProc = HandleMessageSetup;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = m_hInst;
		wcex.hIcon = nullptr;
		wcex.hIconSm = nullptr;
		wcex.hCursor = nullptr;
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = m_className.c_str();

		RegisterClassExA(&wcex);
	}

	Window::WindowClass::~WindowClass()
	{
		UnregisterClassA(m_className.c_str(), m_hInst);
	}




	Window::Window(const WindowDesc& desc)
		: m_windowClass(desc.className), m_desc(desc)
	{
		RECT rect;
		rect.left = 100;
		rect.right = m_desc.width + rect.left;
		rect.top = 100;
		rect.bottom = m_desc.height + rect.top;

		if (AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE) == FALSE)
			throw GDX11_WND_LAST_EXCEPT();

		m_hWnd = CreateWindowExA(
			0,
			m_windowClass.GetName().c_str(),
			m_desc.name.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top,
			nullptr,
			nullptr,
			m_windowClass.GetInstance(),
			this
		);

		if (!m_hWnd)
			throw GDX11_WND_LAST_EXCEPT();

		ShowWindow(m_hWnd, SW_SHOWDEFAULT);
	}

	Window::~Window()
	{
		DestroyWindow(m_hWnd);
	}

	void Window::SetName(const std::string& name)
	{
		m_desc.name = name;
		SetWindowTextA(m_hWnd, name.c_str());
	}

	void Window::Close()
	{
		PostMessageA(m_hWnd, WM_CLOSE, 0, 0);
	}

	void Window::PollEvents()
	{
		MSG msg;
		while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}

	LRESULT Window::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
#ifdef GDX11_IMGUI_SUPPORT
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;
#endif // GDX11_IMGUI_SUPPORT

		switch (msg)
		{
		case WM_CLOSE:
		{
			m_state.shouldClose = true;

			if (!m_eventCallbackFn) return 0;
			WindowCloseEvent e;
			m_eventCallbackFn(e);

			return 0;
		}
		case WM_KILLFOCUS:
		{
			m_state.isFocus = false;
			if (!m_eventCallbackFn) break;
			WindowLostFocusEvent e;
			m_eventCallbackFn(e);
			break;
		}
		case WM_SETFOCUS:
		{
			m_state.isFocus = true;
			if (!m_eventCallbackFn) break;
			WindowFocusEvent e;
			m_eventCallbackFn(e);
			break;
		}
		case WM_SIZE:
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			m_desc.width = width;
			m_desc.height = height;

			if (wParam == SIZE_MINIMIZED || width == 0 || height == 0)
				m_state.isMinimized = true;
			else
				m_state.isMinimized = false;

			if (!m_eventCallbackFn) break;
			WindowResizeEvent e(width, height);
			m_eventCallbackFn(e);
			break;
		}
		/****************************** KEYBOARD MESSAGES *************************************/
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if (!m_eventCallbackFn) break;

			KeyCode key = static_cast<KeyCode>(wParam);
			bool repeat = (lParam & (1 << 30)); // todo figure out how to get repeat count
			KeyPressedEvent e(key, repeat);
			m_eventCallbackFn(e);
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			if (!m_eventCallbackFn) break;

			KeyCode key = static_cast<KeyCode>(wParam);
			KeyReleasedEvent e(key);
			m_eventCallbackFn(e);
			break;
		}
		case WM_CHAR:
		{
			if (!m_eventCallbackFn) break;

			KeyCode c = static_cast<KeyCode>(wParam);
			KeyTypedEvent e(c);
			m_eventCallbackFn(e);
			break;
		}
		/************************** END KEYBOARD MESSAGES *************************************/

		/********************************* MOUSE MESSAGES *************************************/

		case WM_MOUSEMOVE:
		{
			if (!m_eventCallbackFn) break;

			POINTS p = MAKEPOINTS(lParam);
			MouseMovedEvent e(p.x, p.y);
			m_eventCallbackFn(e);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			if (!m_eventCallbackFn) break;
			MouseButtonPressedEvent e(Mouse::LeftButton);
			m_eventCallbackFn(e);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			if (!m_eventCallbackFn) break;
			MouseButtonPressedEvent e(Mouse::RightButton);
			m_eventCallbackFn(e);
			break;
		}
		case WM_MBUTTONDOWN:
		{
			if (!m_eventCallbackFn) break;
			MouseButtonPressedEvent e(Mouse::MiddleButton);
			m_eventCallbackFn(e);
			break;
		}
		case WM_LBUTTONUP:
		{
			if (!m_eventCallbackFn) break;
			MouseButtonReleasedEvent e(Mouse::LeftButton);
			m_eventCallbackFn(e);
			break;
		}
		case WM_RBUTTONUP:
		{
			if (!m_eventCallbackFn) break;
			MouseButtonReleasedEvent e(Mouse::RightButton);
			m_eventCallbackFn(e);
			break;
		}
		case WM_MBUTTONUP:
		{
			if (!m_eventCallbackFn) break;
			MouseButtonReleasedEvent e(Mouse::MiddleButton);
			m_eventCallbackFn(e);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			if (!m_eventCallbackFn) break;

			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			float axisY = 0.0f;
			if (delta >= WHEEL_DELTA)
				axisY = 1.0f;
			else if (delta <= -WHEEL_DELTA)
				axisY = -1.0f;

			MouseScrollEvent e(0.0f, axisY);
			m_eventCallbackFn(e);
			break;
		}
		case WM_MOUSEHWHEEL:
		{
			if (!m_eventCallbackFn) break;

			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			float axisX = 0.0f;
			if (delta >= WHEEL_DELTA)
				axisX = 1.0f;
			else if (delta <= -WHEEL_DELTA)
				axisX = -1.0f;

			MouseScrollEvent e(axisX, 0.0f);
			m_eventCallbackFn(e);
			break;
		}
		}

		return DefWindowProcA(hWnd, msg, wParam, lParam);
	}

	LRESULT WINAPI Window::HandleMessageSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
		if (msg == WM_NCCREATE)
		{
			// extract ptr to window class from creation data
			const CREATESTRUCTA* const createStruct = reinterpret_cast<CREATESTRUCTA*>(lParam);
			Window* const window = static_cast<Window*>(createStruct->lpCreateParams);
			// set WinAPI-managed user data to store ptr to window instance
			SetWindowLongPtrA(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
			// set message proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtrA(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Window::HandleMessageCaller));
			// forward message to window instance handler
			return window->HandleMessage(hWnd, msg, wParam, lParam);
		}

		// if we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProcA(hWnd, msg, wParam, lParam);
	}

	LRESULT WINAPI Window::HandleMessageCaller(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// retrieve ptr to window instance
		Window* const window = reinterpret_cast<Window*>(GetWindowLongPtrA(hWnd, GWLP_USERDATA));
		// forward message to window instance handler
		return window->HandleMessage(hWnd, msg, wParam, lParam);
	}





	std::string Window::Exception::TranslateErrorCode(HRESULT hr)
	{
		char* msgBuffer = nullptr;
		DWORD msgLen = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPSTR>(&msgBuffer), 0, nullptr
		);

		if (msgLen == 0)
			return "Unidentified Error";

		std::string errorString = msgBuffer;
		LocalFree(msgBuffer);

		return errorString;
	}

	Window::HRException::HRException(int line, const std::string& file, HRESULT hr)
		: Exception(line, file), m_hr(hr)
	{
	}

	const char* Window::HRException::what() const
	{
		std::ostringstream oss;
		oss << GetType() << '\n'
			<< "[Error Code] " << GetErrorCode() << '\n'
			<< "[Description] " << GetErrorDescription() << '\n'
			<< GetOriginString();

		m_whatBuffer = oss.str();
		return m_whatBuffer.c_str();
	}
}