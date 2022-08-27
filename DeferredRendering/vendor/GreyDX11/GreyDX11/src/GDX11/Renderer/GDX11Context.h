#pragma once

#include "../Core/NativeWindow.h"
#include "../Core/GDX11Exception.h"
#include "DXError/DxgiInfoManager.h"
#include "../Core/Window.h"

#include <wrl.h>
#include <d3d11.h>

namespace GDX11
{
	class GDX11Context
	{
	public:
		GDX11Context(const DXGI_SWAP_CHAIN_DESC& scDesc);
		GDX11Context(const GDX11Context&) = delete;
		GDX11Context& operator=(const GDX11Context&) = delete;

		~GDX11Context();

		ID3D11Device* const GetDevice() const { return m_device.Get(); }
		ID3D11DeviceContext* const GetDeviceContext() const { return m_deviceContext.Get(); }
		IDXGISwapChain* const GetSwapChain() const { return m_swapChain.Get(); }

#ifdef GDX11_DEBUG
		static DxgiInfoManager& GetInfoManager() { return s_infoManager; }
#endif // GDX11_DEBUG

	private:

#ifdef GDX11_DEBUG
		static DxgiInfoManager s_infoManager;
#endif // GDX11_DEBUG

		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;


		// exception stuffs
	public:
		class Exception : public GDX11Exception
		{
			using GDX11Exception::GDX11Exception;
		};

		class HRException : public Exception
		{
		public:
			HRException(int line, const std::string& file, HRESULT hr, const std::vector<std::string>& infoMessages = {});
			virtual const char* what() const override;
			virtual const char* GetType() const override { return "GDX11Context Exception"; }
			HRESULT GetErrorCode() const { return m_hr; }
			std::string GetErrorString() const;
			std::string GetErrorDescription() const;
			const std::string& GetErrorInfo() const { return m_info; }

		private:
			HRESULT m_hr;
			std::string m_info;
		};

		class InfoException : public Exception
		{
		public:
			InfoException(int line, const std::string& file, const std::vector<std::string>& infoMessages);
			virtual const char* what() const override;
			virtual const char* GetType() const { return "GDX11Context Info Exception"; }
			const std::string& GetErrorInfo() const { return m_info; }

		private:
			std::string m_info;
		};

		class DeviceRemovedException : public HRException
		{
			using HRException::HRException;
		public:
			virtual const char* GetType() const override { return "GDX11Context Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)"; }

		private:
			std::string m_reason;
		};
	};
}


// graphics exception checking/throwing macros (some with dxgi infos)
#define GDX11_CONTEXT_EXCEPT_NOINFO(hr) GDX11::GDX11Context::HRExcetion(__LINE__, __FILE__, (hr))
#define GDX11_CONTEXT_THROW_NOINFO(hrcall) if(FAILED(hr = (hrcall))) throw GDX11::GDX11Context::HRException(__LINE__, __FILE__, hr)

#ifdef GDX11_DEBUG
#define GDX11_CONTEXT_EXCEPT(hr) GDX11::GDX11Context::HRException(__LINE__, __FILE__, (hr), GDX11::GDX11Context::GetInfoManager().GetMessages())
#define GDX11_CONTEXT_THROW_INFO(hrcall) GDX11::GDX11Context::GetInfoManager().Set(); if(FAILED(hr = (hrcall))) throw GDX11_CONTEXT_EXCEPT(hr)
#define GDX11_CONTEXT_DEVICE_REMOVED_EXCEPT(hr) GDX11::GDX11Context::DeviceRemovedException(__LINE__, __FILE__, (hr), GDX11::GDX11Context::GetInfoManager().GetMessages())
#define GDX11_CONTEXT_THROW_INFO_ONLY(call) GDX11::GDX11Context::GetInfoManager().Set(); (call); { auto v = GDX11::GDX11Context::GetInfoManager().GetMessages(); if(!v.empty()) { throw GDX11::GDX11Context::InfoException(__LINE__, __FILE__, v); } }
#else
#define GDX11_CONTEXT_EXCEPT(hr) GDX11::GDX11Context::HRException(__LINE__, __FILE__, (hr))
#define GDX11_CONTEXT_THROW_INFO(hrcall) GDX11_CONTEXT_THROW_NOINFO(hrcall)
#define GDX11_CONTEXT_DEVICE_REMOVED_EXCEPT(hr) GDX11::GDX11Context::DeviceRemovedException(__LINE__, __FILE__, (hr))
#define GDX11_CONTEXT_THROW_INFO_ONLY(call) (call)
#endif // GDX11_DEBUG
