#pragma once

#include <GDX11.h>

class DeferredRendering
{
public:
	DeferredRendering();
	~DeferredRendering() = default;

	void Run();

private:
	void SetImGui();
	void OnEvent(GDX11::Event& event);
	void OnUpdate();
	void OnRender();
	void OnImGuiRender();

	void ImGuiBegin();
	void ImGuiEnd();

	std::unique_ptr<GDX11::Window> m_window;
	std::unique_ptr<GDX11::GDX11Context> m_context;
};