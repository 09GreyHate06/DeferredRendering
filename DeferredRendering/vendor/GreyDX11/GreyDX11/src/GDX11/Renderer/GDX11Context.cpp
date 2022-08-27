#include "GDX11Context.h"

#include "DXError/dxerr.h"
#include "../Core/Log.h"
#include "../Core/GDX11Assert.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace GDX11
{
#ifdef GDX11_DEBUG
	DxgiInfoManager GDX11Context::s_infoManager;
#endif // GDX11_DEBUG

	GDX11Context::GDX11Context(const Window* window, const DXGI_SWAP_CHAIN_DESC& scDesc)
		: GDX11Context(window->GetNativeWindow(), scDesc)
	{
	}

	GDX11Context::GDX11Context(HWND hWnd, const DXGI_SWAP_CHAIN_DESC& scDesc)
	{
		Log::Init();

		GDX11_CORE_ASSERT(IsWindow(hWnd), "Not a window");

		HRESULT hr;

		UINT swapCreateFlags = 0;
#ifdef GDX11_DEBUG
		swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // GDX11_DEBUG

		// create device and front/back buffers, swap chain and rendering context
		GDX11_CONTEXT_THROW_INFO(D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			swapCreateFlags,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&scDesc,
			&m_swapChain,
			&m_device,
			nullptr,
			&m_deviceContext
		));
	}

	GDX11Context::~GDX11Context()
	{
	}




	GDX11Context::GDX11Context::HRException::HRException(int line, const std::string& file, HRESULT hr, const std::vector<std::string>& infoMessages)
		: Exception(line, file), m_hr(hr)
	{
		// join all info messages with newlines into single string
		for (const auto& m : infoMessages)
		{
			m_info += m;
			m_info.push_back('\n');
		}
		// remove final newline if exists
		if (!m_info.empty())
		{
			m_info.pop_back();
		}
	}

	const char* GDX11Context::GDX11Context::HRException::what() const
	{
		std::ostringstream oss;
		oss << GetType() << '\n'
			<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
			<< std::dec << " (" << static_cast<uint32_t>(GetErrorCode()) << ") " << '\n'
			<< "[Error String] " << GetErrorString() << '\n'
			<< "[Description] " << GetErrorDescription() << '\n';
		if (!m_info.empty())
			oss << "\n[Error Info]\n" << GetErrorInfo() << "\n\n";
		oss << GetOriginString();

		m_whatBuffer = oss.str();
		return m_whatBuffer.c_str();
	}

	std::string GDX11Context::GDX11Context::HRException::GetErrorString() const
	{
		return DXGetErrorStringA(m_hr);
	}

	std::string GDX11Context::GDX11Context::HRException::GetErrorDescription() const
	{
		char buffer[512];
		DXGetErrorDescriptionA(m_hr, buffer, sizeof(buffer));
		return buffer;
	}



	GDX11Context::GDX11Context::InfoException::InfoException(int line, const std::string& file, const std::vector<std::string>& infoMessages)
		: Exception(line, file)
	{
		// join all info messages with newlines into single string
		for (const auto& m : infoMessages)
		{
			m_info += m;
			m_info.push_back('\n');
		}
		// remove final newline if exists
		if (!m_info.empty())
		{
			m_info.pop_back();
		}
	}

	const char* GDX11Context::InfoException::what() const
	{
		std::ostringstream oss;
		oss << GetType() << '\n'
			<< "\n[Error Info]\n" << GetErrorInfo() << "\n\n"
			<< GetOriginString();

		m_whatBuffer = oss.str();
		return m_whatBuffer.c_str();
	}
}