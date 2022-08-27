#include "Shader.h"
#include "../Core/GDX11Assert.h"
#include <fstream>

using namespace Microsoft::WRL;

#define GDX11_SHADER_COMPILATION_EXCEPT(hr, msg) Shader::ShaderCompilationException(__LINE__, __FILE__, (hr), (msg))

namespace GDX11
{
	VertexShader::VertexShader(GDX11Context* context, const std::string& src)
		: Shader(context)
	{
		HRESULT hr;
		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(hr = D3DCompile(src.data(), src.size(), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &m_byteCode, &errorBlob)))
			throw GDX11_SHADER_COMPILATION_EXCEPT(hr, static_cast<const char*>(errorBlob->GetBufferPointer()));

		GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateVertexShader(m_byteCode->GetBufferPointer(), m_byteCode->GetBufferSize(), nullptr, &m_vs));

		GDX11_CONTEXT_THROW_INFO(D3DReflect(m_byteCode->GetBufferPointer(), m_byteCode->GetBufferSize(), __uuidof(ID3D11ShaderReflection), &m_reflection));
	}

	void VertexShader::Bind() const
	{
		m_context->GetDeviceContext()->VSSetShader(m_vs.Get(), nullptr, 0);
	}

	uint32_t VertexShader::GetResBinding(const std::string& name)
	{
		if (m_resBindingCache.find(name) != m_resBindingCache.end())
			return m_resBindingCache[name];

		HRESULT hr;
		D3D11_SHADER_INPUT_BIND_DESC desc = {};
		GDX11_CONTEXT_THROW_INFO(m_reflection->GetResourceBindingDescByName(name.c_str(), &desc));
		uint32_t slot = desc.BindPoint;
		m_resBindingCache[name] = slot;
		return slot;
	}

	std::shared_ptr<VertexShader> VertexShader::Create(GDX11Context* context, const std::string& src)
	{
		return std::shared_ptr<VertexShader>(new VertexShader(context, src));
	}


	PixelShader::PixelShader(GDX11Context* context, const std::string& src)
		: Shader(context)
	{
		HRESULT hr;
		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(hr = D3DCompile(src.data(), src.size(), nullptr, nullptr, nullptr, "main", "ps_4_0", 0, 0, &m_byteCode, &errorBlob)))
			throw GDX11_SHADER_COMPILATION_EXCEPT(hr, static_cast<const char*>(errorBlob->GetBufferPointer()));

		GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreatePixelShader(m_byteCode->GetBufferPointer(), m_byteCode->GetBufferSize(), nullptr, &m_ps));
		GDX11_CONTEXT_THROW_INFO(D3DReflect(m_byteCode->GetBufferPointer(), m_byteCode->GetBufferSize(), __uuidof(ID3D11ShaderReflection), &m_reflection));
	}	
	
	PixelShader::PixelShader(GDX11Context* context)
		: Shader(context), m_ps(nullptr), m_byteCode(nullptr), m_reflection(nullptr)
	{
	}

	void PixelShader::Bind() const
	{
		m_context->GetDeviceContext()->PSSetShader(m_ps.Get(), nullptr, 0);
	}

	uint32_t PixelShader::GetResBinding(const std::string& name)
	{
		if (m_resBindingCache.find(name) != m_resBindingCache.end())
			return m_resBindingCache[name];

		HRESULT hr;
		D3D11_SHADER_INPUT_BIND_DESC desc = {};
		GDX11_CONTEXT_THROW_INFO(m_reflection->GetResourceBindingDescByName(name.c_str(), &desc));
		uint32_t slot = desc.BindPoint;
		m_resBindingCache[name] = slot;
		return slot;
	}

	std::shared_ptr<PixelShader> PixelShader::Create(GDX11Context* context, const std::string& src)
	{
		return std::shared_ptr<PixelShader>(new PixelShader(context, src));
	}

	std::shared_ptr<PixelShader> PixelShader::Create(GDX11Context* context)
	{
		return std::shared_ptr<PixelShader>(new PixelShader(context));
	}



	GeometryShader::GeometryShader(GDX11Context* context, const std::string& src)
		: Shader(context)
	{
		HRESULT hr;
		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(hr = D3DCompile(src.data(), src.size(), nullptr, nullptr, nullptr, "main", "gs_4_0", 0, 0, &m_byteCode, &errorBlob)))
			throw GDX11_SHADER_COMPILATION_EXCEPT(hr, static_cast<const char*>(errorBlob->GetBufferPointer()));

		GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateGeometryShader(m_byteCode->GetBufferPointer(), m_byteCode->GetBufferSize(), nullptr, &m_gs));
		GDX11_CONTEXT_THROW_INFO(D3DReflect(m_byteCode->GetBufferPointer(), m_byteCode->GetBufferSize(), __uuidof(ID3D11ShaderReflection), &m_reflection));
	}

	GeometryShader::GeometryShader(GDX11Context* context)
		: Shader(context), m_gs(nullptr), m_byteCode(nullptr), m_reflection(nullptr)
	{
	}

	void GeometryShader::Bind() const
	{
		m_context->GetDeviceContext()->GSSetShader(m_gs.Get(), nullptr, 0);
	}

	uint32_t GeometryShader::GetResBinding(const std::string& name)
	{
		if (m_resBindingCache.find(name) != m_resBindingCache.end())
			return m_resBindingCache[name];

		HRESULT hr;
		D3D11_SHADER_INPUT_BIND_DESC desc = {};
		GDX11_CONTEXT_THROW_INFO(m_reflection->GetResourceBindingDescByName(name.c_str(), &desc));
		uint32_t slot = desc.BindPoint;
		m_resBindingCache[name] = slot;
		return slot;
	}

	std::shared_ptr<GeometryShader> GeometryShader::Create(GDX11Context* context, const std::string& src)
	{
		return std::shared_ptr<GeometryShader>(new GeometryShader(context, src));
	}

	std::shared_ptr<GeometryShader> GeometryShader::Create(GDX11Context* context)
	{
		return std::shared_ptr<GeometryShader>(new GeometryShader(context));
	}

}
