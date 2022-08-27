#include "DeferredRendering.h"

using namespace GDX11;

DeferredRendering::DeferredRendering()
{
	WindowDesc winDesc = {};
	winDesc.width = 1280.0f;
	winDesc.height = 720.0f;
	winDesc.name = "DeferredRendering";
	winDesc.className = "DeferredRenderingClass";
	m_window = std::make_unique<Window>(winDesc);
	m_window->SetEventCallback(GDX11_BIND_EVENT_FN(DeferredRendering::OnEvent));
	
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	scDesc.BufferDesc.Width = 0;
	scDesc.BufferDesc.Height = 0;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.RefreshRate.Numerator = 0;
	scDesc.BufferDesc.RefreshRate.Denominator = 0;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 1;
	scDesc.OutputWindow = m_window->GetNativeWindow();
	scDesc.Windowed = TRUE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = 0;
	m_context = std::make_unique<GDX11Context>(scDesc);

}

void DeferredRendering::Run()
{
}

void DeferredRendering::SetImGui()
{
}

void DeferredRendering::OnEvent(GDX11::Event& event)
{
}

void DeferredRendering::OnUpdate()
{
}

void DeferredRendering::OnRender()
{
}

void DeferredRendering::OnImGuiRender()
{
}

void DeferredRendering::ImGuiBegin()
{
}

void DeferredRendering::ImGuiEnd()
{
}
