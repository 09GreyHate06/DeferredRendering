#pragma once
#include "KeyCodes.h"
#include "MouseCodes.h"
#include "Window.h"

#include <DirectXMath.h>

namespace GDX11
{
	class Input
	{
	public:
		static bool GetKey(const Window* window, const KeyCode key);
		static bool GetKey(HWND window, const KeyCode key);
		static bool GetMouseButton(const Window* window, const MouseCode button);
		static bool GetMouseButton(HWND window, const MouseCode button);
		static DirectX::XMFLOAT2 GetMousePos(const Window* window);
		static DirectX::XMFLOAT2 GetMousePos(HWND window);
		static float GetMouseX(const Window* window);
		static float GetMouseX(HWND window);
		static float GetMouseY(const Window* window);
		static float GetMouseY(HWND window);


	private:
		Input() = default;
	};
}