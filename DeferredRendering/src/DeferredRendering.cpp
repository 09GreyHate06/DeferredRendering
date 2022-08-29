#include "DeferredRendering.h"
#include "Utils/BasicMesh.h"
#include "Utils/CBufs.h"

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

	
	// G-Buffer
	{
		std::shared_ptr<RenderTargetViewArray> rtva = std::make_shared<RenderTargetViewArray>();
		// 0 = pos
		// 1 = normal 
		// 2 = diffuse
		// 3 = specular
		// 4 = shininess
		for (int i = 0; i < 5; i++)
		{
			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.Width = m_window->GetDesc().width;
			texDesc.Height = m_window->GetDesc().height;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;

			if (i < 2)
				texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			else if (i < 4)
				texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			else
				texDesc.Format = DXGI_FORMAT_R32_FLOAT;

			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;
			texDesc.MiscFlags = 0;

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = texDesc.Format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = texDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;

			auto texture = Texture2D::Create(m_context.get(), texDesc, (void*)nullptr);
			rtva->push_back(RenderTargetView::Create(m_context.get(), rtvDesc, texture));
			m_resourceLib.Add("g_bufferTextureView" + std::to_string(i), ShaderResourceView::Create(m_context.get(), srvDesc, texture));
		}

		m_resourceLib.Add("g_buffer", rtva);
	}

	SetShaders();
	SetBuffers();
	SetLoadedTexture();
	SetSamplers();
}

void DeferredRendering::SetShaders()
{
	{
		m_resourceLib.Add("basic", VertexShader::Create(m_context.get(), GDX11::Utils::LoadText("res/shaders/basic.vs.hlsl")));
		m_resourceLib.Add("basic", PixelShader::Create(m_context.get(), GDX11::Utils::LoadText("res/shaders/basic.ps.hlsl")));
		m_resourceLib.Add("basic", InputLayout::Create(m_context.get(), m_resourceLib.Get<VertexShader>("basic")));
	}

	{
		m_resourceLib.Add("g_buffer", VertexShader::Create(m_context.get(), GDX11::Utils::LoadText("res/shaders/g_buffer.vs.hlsl")));
		m_resourceLib.Add("g_buffer", PixelShader::Create(m_context.get(), GDX11::Utils::LoadText("res/shaders/g_buffer.ps.hlsl")));
		m_resourceLib.Add("g_buffer", InputLayout::Create(m_context.get(), m_resourceLib.Get<VertexShader>("g_buffer")));
	}

	{
		m_resourceLib.Add("fullscreen", VertexShader::Create(m_context.get(), GDX11::Utils::LoadText("res/shaders/fullscreen.vs.hlsl")));
		m_resourceLib.Add("fullscreen", PixelShader::Create(m_context.get(), GDX11::Utils::LoadText("res/shaders/fullscreen.ps.hlsl")));
		m_resourceLib.Add("fullscreen", InputLayout::Create(m_context.get(), m_resourceLib.Get<VertexShader>("fullscreen")));
	}

	{
		m_resourceLib.Add("deferred_light", PixelShader::Create(m_context.get(), GDX11::Utils::LoadText("res/shaders/deferred_light.ps.hlsl")));
	}
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
		buffDesc.ByteWidth = sizeof(XMFLOAT4X4);
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


	{
		D3D11_BUFFER_DESC buffDesc = {};
		buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.ByteWidth = sizeof(XMFLOAT4X4);
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = 0;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		m_resourceLib.Add("cbuf.g_buffer.vs.SystemCBuf", Buffer::Create(m_context.get(), buffDesc, nullptr));
	}

	{
		D3D11_BUFFER_DESC buffDesc = {};
		buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.ByteWidth = 2 * sizeof(XMFLOAT4X4);
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = 0;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		m_resourceLib.Add("cbuf.g_buffer.vs.UserCBuf", Buffer::Create(m_context.get(), buffDesc, nullptr));
	}

	{
		D3D11_BUFFER_DESC buffDesc = {};
		buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.ByteWidth = 2 * sizeof(XMFLOAT4);
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = 0;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		m_resourceLib.Add("cbuf.g_buffer.ps.UserCBuf", Buffer::Create(m_context.get(), buffDesc, nullptr));
	}

	{
		D3D11_BUFFER_DESC buffDesc = {};
		buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.ByteWidth = sizeof(CBuf::PS::deferred_lighting::SystemCBuf);
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;
		buffDesc.StructureByteStride = 0;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		m_resourceLib.Add("cbuf.deferred_light.ps.SystemCBuf", Buffer::Create(m_context.get(), buffDesc, nullptr));
	}
}

void DeferredRendering::SetLoadedTexture()
{
	{
		auto data = GDX11::Utils::LoadImageFile("D:/Utilities/Textures/basketball_court_floor.jpg", false, 4);
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = data.width;
		texDesc.Height = data.height;
		texDesc.ArraySize = 1;
		texDesc.MipLevels = 0; // generate mips
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		m_resourceLib.Add("basketball_court", ShaderResourceView::Create(m_context.get(), srvDesc, Texture2D::Create(m_context.get(), texDesc, data.pixels)));

		GDX11::Utils::FreeImageData(&data);
	}
}

void DeferredRendering::SetSamplers()
{
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		for (int i = 0; i < 4; i++)
			samplerDesc.BorderColor[i] = 0.0f;
		samplerDesc.MinLOD = 1.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		m_resourceLib.Add("anisotropic_wrap", SamplerState::Create(m_context.get(), samplerDesc));
	}

	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		for (int i = 0; i < 4; i++)
			samplerDesc.BorderColor[i] = 0.0f;
		samplerDesc.MinLOD = 1.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		m_resourceLib.Add("point_clamp", SamplerState::Create(m_context.get(), samplerDesc));
	}
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
	m_cameraController.OnEvent(event);
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






	// todo: temp
	static CBuf::PS::deferred_lighting::SystemCBuf::DirectionalLight dirLight;
	static CBuf::PS::deferred_lighting::SystemCBuf::PointLight		 pointLights[4];
	static CBuf::PS::deferred_lighting::SystemCBuf::SpotLight		 spotLight;

	XMMATRIX quatMatXM = XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(XMConvertToRadians(50.0f), XMConvertToRadians(-30.0f), 0.0f));
	XMStoreFloat3(&dirLight.direction, XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), quatMatXM));

	pointLights[0].position = {  0.0f,  5.0f, -5.0f };
	pointLights[0].ambient =  {  0.2f,  0.0f,  0.0f };
	pointLights[0].diffuse =  {  1.0f,  0.0f,  0.0f };
	pointLights[0].specular = {  1.0f,  0.0f,  0.0f };

	pointLights[1].position = {  5.0f,  5.0f,  0.0f };
	pointLights[1].ambient =  {  0.0f,  0.2f,  0.0f };
	pointLights[1].diffuse =  {  0.0f,  1.0f,  0.0f };
	pointLights[1].specular = {  0.0f,  1.0f,  0.0f };

	pointLights[2].position = {  0.0f,  5.0f,  5.0f };
	pointLights[2].ambient =  {  0.0f,  0.0f,  0.2f };
	pointLights[2].diffuse =  {  0.0f,  0.0f,  1.0f };
	pointLights[2].specular = {  0.0f,  0.0f,  1.0f };

	pointLights[3].position = { -5.0f,  5.0f,  0.0f };
	pointLights[3].ambient =  {  0.2f,  0.2f,  0.2f };
	pointLights[3].diffuse =  {  1.0f,  1.0f,  1.0f };
	pointLights[3].specular = {  1.0f,  1.0f,  1.0f };


	spotLight.direction = { 0.0f, 1.0f, 0.0f };
	spotLight.position =  { 0.0f,  7.0f, 0.0f };








	m_resourceLib.Get<RasterizerState>("default")->Bind();
	m_resourceLib.Get<BlendState>("default")->Bind(nullptr, 0xffffffff);
	m_resourceLib.Get<DepthStencilState>("default")->Bind(0xff);

	// G-Buffer Pass
	{
		auto gBufferRTV = m_resourceLib.Get<RenderTargetViewArray>("g_buffer");
		RenderTargetView::Bind(*gBufferRTV, m_resourceLib.Get<DepthStencilView>("main").get());
		for (const auto& v : *gBufferRTV)
			v->Clear(0.0f, 0.0f, 0.0f, 0.0f);
		m_resourceLib.Get<DepthStencilView>("main")->Clear(D3D11_CLEAR_DEPTH, 1.0f, 0xff);


		auto vs = m_resourceLib.Get<VertexShader>("g_buffer");
		auto ps = m_resourceLib.Get<PixelShader>("g_buffer");
		vs->Bind();
		ps->Bind();
		m_resourceLib.Get<InputLayout>("g_buffer")->Bind();

		m_resourceLib.Get<Buffer>("cbuf.g_buffer.vs.SystemCBuf")->VSBindAsCBuf(vs->GetResBinding("SystemCBuf"));
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.vs.UserCBuf")->VSBindAsCBuf(vs->GetResBinding("UserCBuf"));
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.ps.UserCBuf")->PSBindAsCBuf(ps->GetResBinding("UserCBuf"));
		

		XMFLOAT4X4 viewProj;
		XMStoreFloat4x4(&viewProj, XMMatrixTranspose(m_camera.GetViewMatrix() * m_camera.GetProjectionMatrix()));
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.vs.SystemCBuf")->SetData(&viewProj);

		DrawScene();
	}

	// render to quad
	{
		m_resourceLib.Get<RenderTargetView>("main")->Bind(nullptr);
		m_resourceLib.Get<RenderTargetView>("main")->Clear(0.1f, 0.1f, 0.1f, 1.0f);


		auto vs = m_resourceLib.Get<VertexShader>("fullscreen");
		auto ps = m_resourceLib.Get<PixelShader>("deferred_light");
		vs->Bind();
		ps->Bind();
		m_resourceLib.Get<InputLayout>("fullscreen")->Bind();

		m_resourceLib.Get<ShaderResourceView>("g_bufferTextureView0")->PSBind(ps->GetResBinding("gPosition"));
		m_resourceLib.Get<ShaderResourceView>("g_bufferTextureView1")->PSBind(ps->GetResBinding("gNormal"));
		m_resourceLib.Get<ShaderResourceView>("g_bufferTextureView2")->PSBind(ps->GetResBinding("gDiffuse"));
		m_resourceLib.Get<ShaderResourceView>("g_bufferTextureView3")->PSBind(ps->GetResBinding("gSpecular"));
		m_resourceLib.Get<ShaderResourceView>("g_bufferTextureView4")->PSBind(ps->GetResBinding("gShininess"));
		m_resourceLib.Get<SamplerState>("point_clamp")->PSBind(ps->GetResBinding("gSampler"));



		// Light
		XMMATRIX quatMatXM = XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(XMConvertToRadians(50.0f), XMConvertToRadians(-30.0f), 0.0f));
		CBuf::PS::deferred_lighting::SystemCBuf psSysCBufData;
		psSysCBufData.dirLights[0] = dirLight;
		std::memcpy(psSysCBufData.pointLights, pointLights, 4 * sizeof(CBuf::PS::deferred_lighting::SystemCBuf::PointLight));
		psSysCBufData.spotLights[0] = spotLight;
		psSysCBufData.activeDirLights = 0;
		psSysCBufData.activePointLights = 4;
		psSysCBufData.activeSpotLights = 1;
		psSysCBufData.viewPosition = m_camera.GetDesc().position;
		auto psSysCBuf = m_resourceLib.Get<Buffer>("cbuf.deferred_light.ps.SystemCBuf");
		psSysCBuf->PSBindAsCBuf(ps->GetResBinding("SystemCBuf"));
		psSysCBuf->SetData(&psSysCBufData);



		m_resourceLib.Get<Buffer>("vb.screen")->BindAsVB();
		auto ib = m_resourceLib.Get<Buffer>("ib.screen");
		ib->BindAsIB(DXGI_FORMAT_R32_UINT);
		m_context->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));
	}

	// forward render light source
	{
		// dont clear dsv we need to get the depth of previous pass
		m_resourceLib.Get<RenderTargetView>("main")->Bind(m_resourceLib.Get<DepthStencilView>("main").get());

		auto vs = m_resourceLib.Get<VertexShader>("basic");
		auto ps = m_resourceLib.Get<PixelShader>("basic");
		vs->Bind();
		ps->Bind();
		m_resourceLib.Get<InputLayout>("basic")->Bind();

		m_resourceLib.Get<Buffer>("cbuf.basic.vs.SystemCBuf")->VSBindAsCBuf(vs->GetResBinding("SystemCBuf"));
		m_resourceLib.Get<Buffer>("cbuf.basic.vs.UserCBuf")->VSBindAsCBuf(vs->GetResBinding("UserCBuf"));
		m_resourceLib.Get<Buffer>("cbuf.basic.ps.UserCBuf")->PSBindAsCBuf(ps->GetResBinding("UserCBuf"));

		XMFLOAT4X4 viewProjection;
		XMStoreFloat4x4(&viewProjection, XMMatrixTranspose(m_camera.GetViewMatrix()* m_camera.GetProjectionMatrix()));
		m_resourceLib.Get<Buffer>("cbuf.basic.vs.SystemCBuf")->SetData(&viewProjection);


		// point lights
		for (int i = 0; i < 4; i++)
		{
			XMMATRIX transformXM =
				XMMatrixScaling(0.25f, 0.25f, 0.25f) *
				XMMatrixTranslation(pointLights[i].position.x, pointLights[i].position.y, pointLights[i].position.z);
			XMFLOAT4X4 transform;
			XMFLOAT4 color = { pointLights[i].diffuse.x, pointLights[i].diffuse.y, pointLights[i].diffuse.z, 1.0f };
			XMStoreFloat4x4(&transform, XMMatrixTranspose(transformXM));
			m_resourceLib.Get<Buffer>("cbuf.basic.vs.UserCBuf")->SetData(&transform);
			m_resourceLib.Get<Buffer>("cbuf.basic.ps.UserCBuf")->SetData(&color);

			DrawCube();
		}

		// spot light
		XMMATRIX transformXM =
			XMMatrixScaling(0.25f, 0.25f, 0.25f) *
			XMMatrixTranslation(spotLight.position.x, spotLight.position.y, spotLight.position.z);
		XMFLOAT4X4 transform;
		XMFLOAT4 color = { spotLight.diffuse.x, spotLight.diffuse.y, spotLight.diffuse.z, 1.0f };
		XMStoreFloat4x4(&transform, XMMatrixTranspose(transformXM));
		m_resourceLib.Get<Buffer>("cbuf.basic.vs.UserCBuf")->SetData(&transform);
		m_resourceLib.Get<Buffer>("cbuf.basic.ps.UserCBuf")->SetData(&color);

		DrawCube();
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

void DeferredRendering::DrawScene()
{
	std::shared_ptr<VertexShader> vs = m_resourceLib.Get<VertexShader>("g_buffer");
	std::shared_ptr<PixelShader> ps = m_resourceLib.Get<PixelShader>("g_buffer");

	// plane 0
	{
		XMVECTOR rotQuatXM = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
		XMMATRIX transformXM =
			XMMatrixScaling(25.0f, 25.0f, 25.0f) *
			XMMatrixRotationQuaternion(rotQuatXM) *
			XMMatrixTranslation(0.0f, -0.5f, 0.0f);

		XMFLOAT4X4 transformNormalMatrix[2];
		XMStoreFloat4x4(&transformNormalMatrix[0], XMMatrixTranspose(transformXM));
		XMStoreFloat4x4(&transformNormalMatrix[1], XMMatrixInverse(nullptr, transformXM));
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.vs.UserCBuf")->SetData(transformNormalMatrix);

		XMFLOAT4 material[2];
		material[0] = { 1.0f, 1.0f, 1.0f, 1.0f }; // diffuseCol
		material[1] = { 10.0f, 10.0f, 120.0f, 0.0f }; // tiling.xy / shininess.z / padding.w
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.ps.UserCBuf")->SetData(material);

		m_resourceLib.Get<ShaderResourceView>("basketball_court")->PSBind(ps->GetResBinding("diffuseMap"));
		m_resourceLib.Get<ShaderResourceView>("basketball_court")->PSBind(ps->GetResBinding("specularMap"));
		m_resourceLib.Get<SamplerState>("anisotropic_wrap")->PSBind(ps->GetResBinding("diffuseMapSampler"));
		m_resourceLib.Get<SamplerState>("anisotropic_wrap")->PSBind(ps->GetResBinding("specularMapSampler"));

		DrawPlane();
	}




	//  cube 0
	{
		XMVECTOR rotQuatXM = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
		XMMATRIX transformXM =
			XMMatrixScaling(1.0f, 1.0f, 1.0f) *
			XMMatrixRotationQuaternion(rotQuatXM) *
			XMMatrixTranslation(0.0f, 1.5f, 0.0f);

		XMFLOAT4X4 transformNormalMatrix[2];
		XMStoreFloat4x4(&transformNormalMatrix[0], XMMatrixTranspose(transformXM));
		XMStoreFloat4x4(&transformNormalMatrix[1], XMMatrixInverse(nullptr, transformXM));
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.vs.UserCBuf")->SetData(transformNormalMatrix);

		XMFLOAT4 material[2];
		material[0] = { 1.0f, 1.0f, 1.0f, 1.0f }; // diffuseCol
		material[1] = { 1.0f, 1.0f, 32.0f, 0.0f }; // tiling.xy / shininess.z / padding.w
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.ps.UserCBuf")->SetData(material);

		m_resourceLib.Get<ShaderResourceView>("basketball_court")->PSBind(ps->GetResBinding("diffuseMap"));
		m_resourceLib.Get<ShaderResourceView>("basketball_court")->PSBind(ps->GetResBinding("specularMap"));
		m_resourceLib.Get<SamplerState>("anisotropic_wrap")->PSBind(ps->GetResBinding("diffuseMapSampler"));
		m_resourceLib.Get<SamplerState>("anisotropic_wrap")->PSBind(ps->GetResBinding("specularMapSampler"));

		DrawCube();
	}



	// cube 1 
	{
		XMVECTOR rotQuatXM = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
		XMMATRIX transformXM =
			XMMatrixScaling(1.0f, 1.0f, 1.0f) *
			XMMatrixRotationQuaternion(rotQuatXM) *
			XMMatrixTranslation(2.0f, 0.0f, -1.0f);

		XMFLOAT4X4 transformNormalMatrix[2];
		XMStoreFloat4x4(&transformNormalMatrix[0], XMMatrixTranspose(transformXM));
		XMStoreFloat4x4(&transformNormalMatrix[1], XMMatrixInverse(nullptr, transformXM));
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.vs.UserCBuf")->SetData(transformNormalMatrix);

		XMFLOAT4 material[2];
		material[0] = { 1.0f, 1.0f, 1.0f, 1.0f }; // diffuseCol
		material[1] = { 1.0f, 1.0f, 32.0f, 0.0f }; // tiling.xy / shininess.z / padding.w
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.ps.UserCBuf")->SetData(material);

		m_resourceLib.Get<ShaderResourceView>("basketball_court")->PSBind(ps->GetResBinding("diffuseMap"));
		m_resourceLib.Get<ShaderResourceView>("basketball_court")->PSBind(ps->GetResBinding("specularMap"));
		m_resourceLib.Get<SamplerState>("anisotropic_wrap")->PSBind(ps->GetResBinding("diffuseMapSampler"));
		m_resourceLib.Get<SamplerState>("anisotropic_wrap")->PSBind(ps->GetResBinding("specularMapSampler"));

		DrawCube();
	}



	// cube 2 
	{
		XMVECTOR rotQuatXM = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(60.0f), XMConvertToRadians(60.0f), XMConvertToRadians(60.0f));
		XMMATRIX transformXM =
			XMMatrixScaling(0.5f, 0.5f, 0.5f) *
			XMMatrixRotationQuaternion(rotQuatXM) *
			XMMatrixTranslation(-1.0f, 0.0f, -2.0f);

		XMFLOAT4X4 transformNormalMatrix[2];
		XMStoreFloat4x4(&transformNormalMatrix[0], XMMatrixTranspose(transformXM));
		XMStoreFloat4x4(&transformNormalMatrix[1], XMMatrixInverse(nullptr, transformXM));
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.vs.UserCBuf")->SetData(transformNormalMatrix);

		XMFLOAT4 material[2];
		material[0] = { 1.0f, 1.0f, 1.0f, 1.0f }; // diffuseCol
		material[1] = { 1.0f, 1.0f, 32.0f, 0.0f }; // tiling.xy / shininess.z / padding.w
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.ps.UserCBuf")->SetData(material);

		m_resourceLib.Get<ShaderResourceView>("basketball_court")->PSBind(ps->GetResBinding("diffuseMap"));
		m_resourceLib.Get<ShaderResourceView>("basketball_court")->PSBind(ps->GetResBinding("specularMap"));
		m_resourceLib.Get<SamplerState>("anisotropic_wrap")->PSBind(ps->GetResBinding("diffuseMapSampler"));
		m_resourceLib.Get<SamplerState>("anisotropic_wrap")->PSBind(ps->GetResBinding("specularMapSampler"));

		DrawCube();
	}


	// cube 3
	{
		XMVECTOR rotQuatXM = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
		XMMATRIX transformXM =
			XMMatrixScaling(0.25f, 0.25f, 0.25f) *
			XMMatrixRotationQuaternion(rotQuatXM) *
			XMMatrixTranslation(1.5f, 4.0f, -2.0f);

		XMFLOAT4X4 transformNormalMatrix[2];
		XMStoreFloat4x4(&transformNormalMatrix[0], XMMatrixTranspose(transformXM));
		XMStoreFloat4x4(&transformNormalMatrix[1], XMMatrixInverse(nullptr, transformXM));
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.vs.UserCBuf")->SetData(transformNormalMatrix);

		XMFLOAT4 material[2];
		material[0] = { 1.0f, 1.0f, 1.0f, 1.0f }; // diffuseCol
		material[1] = { 1.0f, 1.0f, 32.0f, 0.0f }; // tiling.xy / shininess.z / padding.w
		m_resourceLib.Get<Buffer>("cbuf.g_buffer.ps.UserCBuf")->SetData(material);

		m_resourceLib.Get<ShaderResourceView>("basketball_court")->PSBind(ps->GetResBinding("diffuseMap"));
		m_resourceLib.Get<ShaderResourceView>("basketball_court")->PSBind(ps->GetResBinding("specularMap"));
		m_resourceLib.Get<SamplerState>("anisotropic_wrap")->PSBind(ps->GetResBinding("diffuseMapSampler"));
		m_resourceLib.Get<SamplerState>("anisotropic_wrap")->PSBind(ps->GetResBinding("specularMapSampler"));

		DrawCube();
	}
}

void DeferredRendering::DrawCube()
{
	m_resourceLib.Get<Buffer>("vb.cube")->BindAsVB();
	auto ib = m_resourceLib.Get<Buffer>("ib.cube");;
	ib->BindAsIB(DXGI_FORMAT_R32_UINT);
	m_context->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));
}

void DeferredRendering::DrawPlane()
{
	m_resourceLib.Get<Buffer>("vb.plane")->BindAsVB();
	auto ib = m_resourceLib.Get<Buffer>("ib.plane");;
	ib->BindAsIB(DXGI_FORMAT_R32_UINT);
	m_context->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));
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







	// G-Buffer
	{
		m_resourceLib.Remove<RenderTargetViewArray>("g_buffer");
		for (int i = 0; i < 5; i++)
			m_resourceLib.Remove<ShaderResourceView>("g_bufferTextureView" + std::to_string(i));

		std::shared_ptr<RenderTargetViewArray> rtva = std::make_shared<RenderTargetViewArray>();
		// 0 = pos
		// 1 = normal 
		// 2 = diffuse
		// 3 = specular
		// 4 = shininess
		for (int i = 0; i < 5; i++)
		{
			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;

			if (i < 2)
				texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			else if (i < 4)
				texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			else
				texDesc.Format = DXGI_FORMAT_R32_FLOAT;

			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;
			texDesc.MiscFlags = 0;

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = texDesc.Format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = texDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;

			auto texture = Texture2D::Create(m_context.get(), texDesc, (void*)nullptr);
			rtva->push_back(RenderTargetView::Create(m_context.get(), rtvDesc, texture));
			m_resourceLib.Add("g_bufferTextureView" + std::to_string(i), ShaderResourceView::Create(m_context.get(), srvDesc, texture));
		}

		m_resourceLib.Add("g_buffer", rtva);
	}







	m_camera.SetAspect((float)m_window->GetDesc().width / (float)m_window->GetDesc().height);
}
