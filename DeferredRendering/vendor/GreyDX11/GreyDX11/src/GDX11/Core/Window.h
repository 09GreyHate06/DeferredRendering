#pragma once
#include <functional>

#include "NativeWindow.h"
#include "GDX11Exception.h"
#include "../Event/Event.h"

namespace GDX11
{
	struct WindowDesc
	{
		uint32_t width = 1280;
		uint32_t height = 720;
		std::string name = "GDX11";
		std::string className = name;

		WindowDesc() = default;
		WindowDesc(uint32_t width, uint32_t height, const std::string& name, const std::string& className)
			: width(width), height(height), name(name), className(className) { }
	};

	struct WindowState
	{
		bool shouldClose = false;
		bool isMinimized = false;
		bool isFocus = true;

		WindowState() = default;
		WindowState(bool shouldClose, bool isMinimized, bool isFocus)
			: shouldClose(shouldClose), isMinimized(isMinimized), isFocus(isFocus) { }
	};

	class Window
	{
		using EventCallbackFn = std::function<void(Event&)>;

	private:
		class WindowClass
		{
		public:
			WindowClass(const std::string& windowClassName);
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;

			const std::string& GetName() const { return m_className; }
			HINSTANCE GetInstance() const { return m_hInst; }

		private:
			const std::string m_className;
			HINSTANCE m_hInst;
		};

	public:
		Window(const WindowDesc& desc);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		void SetName(const std::string& title);
		inline void SetEventCallback(const EventCallbackFn& callback) { m_eventCallbackFn = callback; }

		const WindowDesc& GetDesc() const { return m_desc; }
		const WindowState& GetState() const { return m_state; }
		HWND GetNativeWindow() const { return m_hWnd; }

		void Close();


		static void PollEvents();

		operator HWND()
		{
			return m_hWnd;
		}

	private:
		LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static LRESULT WINAPI HandleMessageSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static LRESULT WINAPI HandleMessageCaller(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		WindowDesc m_desc;
		WindowState m_state;
		WindowClass m_windowClass;
		HWND m_hWnd;
		EventCallbackFn m_eventCallbackFn;




		// Exceptions stuff
	public:
		class Exception : public GDX11Exception
		{
			using GDX11Exception::GDX11Exception;
		public:
			static std::string TranslateErrorCode(HRESULT hr);
		};

		class HRException : public Exception
		{
		public:
			HRException(int line, const std::string& file, HRESULT hr);
			virtual const char* what() const override;

			HRESULT GetErrorCode() const { return m_hr; }
			std::string GetErrorDescription() const { return TranslateErrorCode(m_hr); }
			virtual const char* GetType() const override { return "Window Exception"; }

		private:
			HRESULT m_hr;
		};
	};
}

#define GDX11_WND_EXCEPT(hr) GDX11::Window::HRException(__LINE__, __FILE__, (hr))
#define GDX11_WND_LAST_EXCEPT() GDX11_WND_EXCEPT(GetLastError())