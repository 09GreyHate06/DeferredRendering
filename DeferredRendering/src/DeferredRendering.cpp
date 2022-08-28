#include "DeferredRendering.h"
#include "Utils/BasicMesh.h"
#include <DirectXMath.h>
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <GDX11/Utils/Loader.h>

using namespace GDX11;
using namespace DRUtils;
using namespace DirectX;
using namespace Microsoft::WRL;

DeferredRendering::DeferredRendering()
{
	WindowDesc winDesc = {};
	winDesc.width = 1280;
	winDesc.height = 720;
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

	CameraDesc camDesc = {};
	camDesc.position = { 0.0f, 12.0f, -7.0f };
	camDesc.aspect = 1280.0f / 720.0f;
	camDesc.fov = XMConvertToRadians(45.0f);
	camDesc.farPlane = 1000.0f;
	camDesc.nearPlane = 0.1f;
	camDesc.pitch = XMConvertToRadians(30.0f);
	camDesc.yaw = 0.0f;
	m_camera.Set(camDesc);

	m_cameraController.SetContext(&m_camera);
	m_cameraController.SetDistance(10.0f);
	m_cameraController.SetFocalPoint({ 0.0f, 0.0f, 0.0f });
	m_cameraController.SetViewportSize((float)m_window->GetDesc().width, (float)m_window->GetDesc().height);

	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)m_window->GetDesc().width;
	vp.Height = (float)m_window->GetDesc().height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_context->GetDeviceContext()->RSSetViewports(1, &vp);

	SetImGui();
	SetResources();
}

void DeferredRendering::Run()
{
	while (!m_window->GetState().shouldClose)
	{
		if (GDX11::Input::GetKey(m_window.get(), GDX11::Key::Escape))
		{
			m_window->Close();
			continue;
		}

		OnUpdate();
		OnRender();

		ImGuiBegin();
		OnImGuiRender();
		ImGuiEnd();

		GDX11::Window::PollEvents();
		HRESULT hr;
		if (FAILED(hr = m_context->GetSwapChain()->Present(1, 0)))
		{
			if (hr == DXGI_ERROR_DEVICE_REMOVED)
				throw GDX11_CONTEXT_DEVICE_REMOVED_EXCEPT(hr);
			else
				throw GDX11_CONTEXT_EXCEPT(hr);
		}
	}
}

void DeferredRendering::SetResources()
{
	HRESULT hr;

	{
		ComPtr<ID3D11Texture2D> backBuffer;
		GDX11_CONTEXT_THROW_INFO(m_context->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		m_resourceLib.Add("main", RenderTargetView::Create(m_context.get(), rtvDesc, Texture2D::Create(m_context.get(), backBuffer.Get())));
	}

	{
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = m_window->GetDesc().width;
		texDesc.Height = m_window->GetDesc().height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_D32_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = texDesc.Format;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		m_resourceLib.Add("main", DepthStencilView::Create(m_context.get(), dsvDesc, Texture2D::Create(m_context.get(), texDesc, (void*)nullptr)));
	}

	m_resourceLib.Add("default", RasterizerState::Create(m_context.get(), CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT())));
	m_resourceLib.Add("default", DepthStencilState::Create(m_context.get(), CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT())));
	m_resourceLib.Add("default", BlendState::Create(m_context.get(), CD3D11_BLEND_DESC(CD3D11_DEFAULT())));


	SetShaders();
	SetBuffers();
	SetLoadedTexture();
	SetSamplers();
}

void DeferredRendering::SetShaders()
{
	m_resourceLib.Add("basic", VertexShader::Create(m_context.get(), GDX11::Utils::LoadText("res/shaders/basic.vs.hlsl")));
	m_resourceLib.Add("basic", PixelShader::Create(m_context.get(), GDX11::Utils::LoadText("res/shaders/basic.ps.hlsl")));
	m_resourceLib.Add("basic", InputLayout::Create(m_context.get(), m_resourceLib.Get<VertexShader>("basic")));
}

void DeferredRendering::SetBuffers()
{
	{
		auto cubeVert = DRUtils::BasicMesh::CreateCubeVertices(true, true);
		auto cubeInd = DRUtils::BasicMesh::CreateCubeIndices();

		D3D11_BUFFER_DESC buffDesc = {};
		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.ByteWidth = (uint32_t)cubeVert.size() * sizeof(float);
		buffDesc.CPUAccessFlags = 0;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = 8 * sizeof(float);
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		m_resourceLib.Add("vb.cube", Buffer::Create(m_context.get(), buffDesc, cubeVert.data()));

		buffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffDesc.ByteWidth = (uint32_t)cubeInd.size() * sizeof(float);
		buffDesc.CPUAccessFlags = 0;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = sizeof(uint32_t);
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		m_resourceLib.Add("ib.cube", Buffer::Create(m_context.get(), buffDesc, cubeInd.data()));
	}


	{
		auto planeVert = DRUtils::BasicMesh::CreatePlaneVertices(true, true);
		auto planeInd = DRUtils::BasicMesh::CreatePlaneIndices();

		D3D11_BUFFER_DESC buffDesc = {};
		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.ByteWidth = (uint32_t)planeVert.size() * sizeof(float);
		buffDesc.CPUAccessFlags = 0;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = 8 * sizeof(float);
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		m_resourceLib.Add("vb.plane", Buffer::Create(m_context.get(), buffDesc, planeVert.data()));

		buffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffDesc.ByteWidth = (uint32_t)planeInd.size() * sizeof(uint32_t);
		buffDesc.CPUAccessFlags = 0;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = sizeof(uint32_t);
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		m_resourceLib.Add("ib.plane", Buffer::Create(m_context.get(), buffDesc, planeInd.data()));
	}


	{
		std::array<float, 8> screenVert = {
		-1.0f,  1.0f,
		 1.0f,  1.0f,
		 1.0f, -1.0f,
		-1.0f, -1.0f
		};
		std::array<uint32_t, 6> screenInd = {
			0, 1, 2,
			2, 3, 0
		};

		D3D11_BUFFER_DESC buffDesc = {};
		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.ByteWidth = (uint32_t)screenVert.size() * sizeof(float);
		buffDesc.CPUAccessFlags = 0;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = 2 * sizeof(float);
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		m_resourceLib.Add("vb.screen", Buffer::Create(m_context.get(), buffDesc, screenVert.data()));

		buffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffDesc.ByteWidth = (uint32_t)screenInd.size() * sizeof(uint32_t);
		buffDesc.CPUAccessFlags = 0;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = sizeof(uint32_t);
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		m_resourceLib.Add("ib.screen", Buffer::Create(m_context.get(), buffDesc, screenInd.data()));
	}

	{
		D3D11_BUFFER_DESC buffDesc = {};
		buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.ByteWidth = 2 * sizeof(XMFLOAT4X4);
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = 0;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		m_resourceLib.Add("cbuf.basic.vs.SystemCBuf", Buffer::Create(m_context.get(), buffDesc, nullptr));
	}

	{
		D3D11_BUFFER_DESC buffDesc = {};
		buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.ByteWidth = sizeof(XMFLOAT4X4);
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = 0;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		m_resourceLib.Add("cbuf.basic.vs.UserCBuf", Buffer::Create(m_context.get(), buffDesc, nullptr));
	}

	{
		D3D11_BUFFER_DESC buffDesc = {};
		buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.ByteWidth = sizeof(XMFLOAT4);
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = 0;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		m_resourceLib.Add("cbuf.basic.ps.UserCBuf", Buffer::Create(m_context.get(), buffDesc, nullptr));
	}
}

void DeferredRendering::SetLoadedTexture()
{
}

void DeferredRendering::SetSamplers()
{
}

void DeferredRendering::SetImGui()
{
	// Setup ImGui Context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable Multi-Viewport / Platform Windows
	io.WantCaptureMouse = true;
	io.WantCaptureKeyboard = true;

	//float fontSize = 18.0f;// *2.0f;
	//io.Fonts->AddFontFromFileTTF("res/fonts/opensans/OpenSans-Bold.ttf", fontSize);
	//io.FontDefault = io.Fonts->AddFontFromFileTTF("res/fonts/opensans/OpenSans-Regular.ttf", fontSize);

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(m_window->GetNativeWindow());
	ImGui_ImplDX11_Init(m_context->GetDevice(), m_context->GetDeviceContext());
}

void DeferredRendering::OnEvent(GDX11::Event& event)
{
	EventDispatcher e(event);
	e.Dispatch<WindowResizeEvent>(GDX11_BIND_EVENT_FN(DeferredRendering::OnWindowResizedEvent));

	if (event.handled) return;

	// events
}

void DeferredRendering::OnUpdate()
{
	m_cameraController.OnUpdate(m_window.get());
}

void DeferredRendering::OnRender()
{
	if (m_resize)
	{
		ResizeResources(m_window->GetDesc().width, m_window->GetDesc().height);
		m_resize = false;
	}



	{
		m_resourceLib.Get<RenderTargetView>("main")->Bind(m_resourceLib.Get<DepthStencilView>("main").get());
		m_resourceLib.Get<RenderTargetView>("main")->Clear(0.1f, 0.1f, 0.1f, 1.0f);
		m_resourceLib.Get<DepthStencilView>("main")->Clear(D3D11_CLEAR_DEPTH, 1.0f, 0xff);

		m_resourceLib.Get<RasterizerState>("default")->Bind();
		m_resourceLib.Get<BlendState>("default")->Bind(nullptr, 0xffffffff);
		m_resourceLib.Get<DepthStencilState>("default")->Bind(0xff);

		auto vs = m_resourceLib.Get<VertexShader>("basic");
		auto ps = m_resourceLib.Get<PixelShader>("basic");
		vs->Bind();
		ps->Bind();
		m_resourceLib.Get<InputLayout>("basic")->Bind();

		m_resourceLib.Get<Buffer>("cbuf.basic.vs.SystemCBuf")->VSBindAsCBuf(vs->GetResBinding("SystemCBuf"));
		m_resourceLib.Get<Buffer>("cbuf.basic.vs.UserCBuf")->VSBindAsCBuf(vs->GetResBinding("UserCBuf"));
		m_resourceLib.Get<Buffer>("cbuf.basic.ps.UserCBuf")->PSBindAsCBuf(ps->GetResBinding("UserCBuf"));
		

		XMFLOAT4X4 viewProj;
		XMFLOAT4X4 transform;
		XMStoreFloat4x4(&viewProj, XMMatrixTranspose(m_camera.GetViewMatrix() * m_camera.GetProjectionMatrix()));
		XMStoreFloat4x4(&transform, XMMatrixTranspose(XMMatrixTranslation(0.0f, 0.0f, 0.0f)));
		XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_resourceLib.Get<Buffer>("cbuf.basic.vs.SystemCBuf")->SetData(&viewProj);
		m_resourceLib.Get<Buffer>("cbuf.basic.vs.UserCBuf")->SetData(&transform);
		m_resourceLib.Get<Buffer>("cbuf.basic.ps.UserCBuf")->SetData(&color);


		m_resourceLib.Get<Buffer>("vb.cube")->BindAsVB();
		auto ib = m_resourceLib.Get<Buffer>("ib.cube");;
		ib->BindAsIB(DXGI_FORMAT_R32_UINT);
		m_context->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));
	}
}

void DeferredRendering::OnImGuiRender()
{

}



void DeferredRendering::ImGuiBegin()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void DeferredRendering::ImGuiEnd()
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(m_window->GetDesc().width), static_cast<float>((m_window->GetDesc().height)));

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

bool DeferredRendering::OnWindowResizedEvent(GDX11::WindowResizeEvent& e)
{
	m_resize = true;
	return false;
}

void DeferredRendering::ResizeResources(uint32_t width, uint32_t height)
{

	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_context->GetDeviceContext()->RSSetViewports(1, &vp);


	m_resourceLib.Remove<RenderTargetView>("main");
	m_resourceLib.Remove<DepthStencilView>("main");


	HRESULT hr;
	GDX11_CONTEXT_THROW_INFO(m_context->GetSwapChain()->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ComPtr<ID3D11Texture2D> backBuffer = nullptr;
	GDX11_CONTEXT_THROW_INFO(m_context->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	m_resourceLib.Add("main", RenderTargetView::Create(m_context.get(), rtvDesc, Texture2D::Create(m_context.get(), backBuffer.Get())));


	D3D11_TEXTURE2D_DESC dsvTexDesc = {};
	dsvTexDesc.Width = width;
	dsvTexDesc.Height = height;
	dsvTexDesc.MipLevels = 1;
	dsvTexDesc.ArraySize = 1;
	dsvTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvTexDesc.SampleDesc.Count = 1;
	dsvTexDesc.SampleDesc.Quality = 0;
	dsvTexDesc.Usage = D3D11_USAGE_DEFAULT;
	dsvTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsvTexDesc.CPUAccessFlags = 0;
	dsvTexDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = dsvTexDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	m_resourceLib.Add("main", DepthStencilView::Create(m_context.get(), dsvDesc, Texture2D::Create(m_context.get(), dsvTexDesc, (void*)nullptr)));


	m_camera.SetAspect((float)m_window->GetDesc().width / (float)m_window->GetDesc().height);
}
