#pragma once
#include "RenderingResource.h"
#include <wrl.h>
#include <d3dcompiler.h>

namespace GDX11
{
	template<class T>
	class Shader : public RenderingResource<T>
	{
	public:
		using RenderingResource<T>::RenderingResource;

		virtual ~Shader() = default;

		virtual void Bind() const = 0;

		virtual uint32_t GetResBinding(const std::string& name) = 0;
		virtual ID3DBlob* GetByteCode() const = 0;
		virtual ID3D11ShaderReflection* GetReflection() const = 0;

		// exception
	public:
		class Exception : public GDX11::GDX11Exception
		{
			using GDX11::GDX11Exception::GDX11Exception;
		};

		class ShaderCompilationException : public Exception
		{
		public:
			ShaderCompilationException(int line, const std::string& file, HRESULT hr, const std::string& info)
				: Exception(line, file), m_hr(hr), m_info(info)
			{
			}

			virtual const char* what() const override
			{
				std::ostringstream oss;
				oss << GetType() << '\n'
					<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
					<< std::dec << " (" << static_cast<uint32_t>(GetErrorCode()) << ") " << '\n'
					<< "[Error Info]: " << GetErrorInfo() << '\n';
				oss << GetOriginString();

				m_whatBuffer = oss.str();
				return m_whatBuffer.c_str();
			}

			virtual const char* GetType() const override { return "Shader Compilation Exception"; }
			const std::string& GetErrorInfo() const { return m_info; }
			HRESULT GetErrorCode() const { return m_hr; }

		private:
			std::string m_info;
			HRESULT m_hr;
		};
	};

	class VertexShader : Shader<ID3D11VertexShader>
	{
	public:
		virtual ~VertexShader() = default;

		virtual void Bind() const;
		virtual uint32_t GetResBinding(const std::string& name);

		virtual ID3D11VertexShader* GetNative() const override { return m_vs.Get(); }
		virtual ID3DBlob* GetByteCode() const override { return m_byteCode.Get(); }
		virtual ID3D11ShaderReflection* GetReflection() const override { return m_reflection.Get(); }

		static std::shared_ptr<VertexShader> Create(GDX11Context* context, const std::string& src);

	private:
		VertexShader(GDX11Context* context, const std::string& src);

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_byteCode;
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> m_reflection;

		std::unordered_map<std::string, uint32_t> m_resBindingCache;
	};

	class PixelShader : public Shader<ID3D11PixelShader>
	{
	public:
		virtual ~PixelShader() = default;

		virtual void Bind() const;
		virtual uint32_t GetResBinding(const std::string& name);

		virtual ID3D11PixelShader* GetNative() const override { return m_ps.Get(); }
		virtual ID3DBlob* GetByteCode() const override { return m_byteCode.Get(); }
		virtual ID3D11ShaderReflection* GetReflection() const override { return m_reflection.Get(); }

		static std::shared_ptr<PixelShader> Create(GDX11Context* context, const std::string& src);
		static std::shared_ptr<PixelShader> Create(GDX11Context* context);

	private:
		PixelShader(GDX11Context* context, const std::string& src);
		PixelShader(GDX11Context* context);

		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps;
		Microsoft::WRL::ComPtr<ID3DBlob> m_byteCode;
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> m_reflection;

		std::unordered_map<std::string, uint32_t> m_resBindingCache;
	};

	class GeometryShader : public Shader<ID3D11GeometryShader>
	{
	public:
		virtual ~GeometryShader() = default;

		virtual void Bind() const;
		virtual uint32_t GetResBinding(const std::string& name);

		virtual ID3D11GeometryShader* GetNative() const override { return m_gs.Get(); }
		virtual ID3DBlob* GetByteCode() const override { return m_byteCode.Get(); }
		virtual ID3D11ShaderReflection* GetReflection() const override { return m_reflection.Get(); }

		static std::shared_ptr<GeometryShader> Create(GDX11Context* context, const std::string& src);
		static std::shared_ptr<GeometryShader> Create(GDX11Context* context);

	private:
		GeometryShader(GDX11Context* context, const std::string& src);
		GeometryShader(GDX11Context* context);

		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_gs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_byteCode;
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> m_reflection;

		std::unordered_map<std::string, uint32_t> m_resBindingCache;
	};
}