#pragma once

#include <GDX11.h>
#include "Utils/ResourceLibrary.h"
#include "Utils/Camera.h"
#include "Utils/CameraController.h"


class DeferredRendering
{
public:
	DeferredRendering();
	~DeferredRendering() = default;

	void Run();

private:
	void SetResources();
	void SetShaders();
	void SetBuffers();
	void SetLoadedTexture();
	void SetSamplers();

	void SetImGui();
	void OnEvent(GDX11::Event& event);
	void OnUpdate();
	void OnRender();
	void OnImGuiRender();

	void ImGuiBegin();
	void ImGuiEnd();


	bool OnWindowResizedEvent(GDX11::WindowResizeEvent& e);
	void ResizeResources(uint32_t width, uint32_t height);

	bool m_resize = false;

	std::unique_ptr<GDX11::Window> m_window;
	std::unique_ptr<GDX11::GDX11Context> m_context;
	DRUtils::ResourceLibrary m_resourceLib;
	DRUtils::Camera m_camera;
	DRUtils::CameraController m_cameraController;
};