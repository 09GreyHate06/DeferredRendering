#include "ShaderResourceView.h"

namespace GDX11
{
	ShaderResourceView::ShaderResourceView(GDX11Context* context, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, const std::shared_ptr<Texture2D>& tex)
		: RenderingResource(context), m_texture(tex)
	{
		GDX11_CORE_ASSERT(m_texture, "Texture2D is null");

		D3D11_TEXTURE2D_DESC texDesc = {};
		m_texture->GetNative()->GetDesc(&texDesc);
		GDX11_CORE_ASSERT(texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE, "tex is not a shader resource");

		HRESULT hr;
		GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateShaderResourceView(m_texture->GetNative(), &srvDesc, &m_srv));

		if (texDesc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
			m_context->GetDeviceContext()->GenerateMips(m_srv.Get());
	}

	ShaderResourceView::ShaderResourceView(GDX11Context* context, ID3D11ShaderResourceView* srv, const std::shared_ptr<Texture2D>& tex)
		: RenderingResource(context), m_srv(srv), m_texture(tex)
	{
		GDX11_CORE_ASSERT(m_srv, "ShaderResourceView is null");
	}

	ShaderResourceView::ShaderResourceView(GDX11Context* context, ID3D11ShaderResourceView* srv, ID3D11Texture2D* tex)
		: ShaderResourceView(context, srv, Texture2D::Create(context, tex))
	{
		GDX11_CORE_ASSERT(m_texture, "Texture2D is null");

		D3D11_TEXTURE2D_DESC texDesc = {};
		m_texture->GetNative()->GetDesc(&texDesc);
		GDX11_CORE_ASSERT(texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE, "tex is not a shader resource");
	}

	void ShaderResourceView::VSBind(uint32_t slot) const
	{
		m_context->GetDeviceContext()->VSSetShaderResources(slot, 1, m_srv.GetAddressOf());
	}

	void ShaderResourceView::GSBind(uint32_t slot) const
	{
		m_context->GetDeviceContext()->GSSetShaderResources(slot, 1, m_srv.GetAddressOf());
	}

	void ShaderResourceView::PSBind(uint32_t slot) const
	{
		m_context->GetDeviceContext()->PSSetShaderResources(slot, 1, m_srv.GetAddressOf());
	}

	std::shared_ptr<ShaderResourceView> GDX11::ShaderResourceView::Create(GDX11Context* context, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, const std::shared_ptr<Texture2D>& tex)
	{
		return std::shared_ptr<ShaderResourceView>(new ShaderResourceView(context, srvDesc, tex));
	}

	std::shared_ptr<ShaderResourceView> ShaderResourceView::Create(GDX11Context* context, ID3D11ShaderResourceView* srv, const std::shared_ptr<Texture2D>& tex)
	{
		return std::shared_ptr<ShaderResourceView>(new ShaderResourceView(context, srv, tex));
	}

	std::shared_ptr<ShaderResourceView> ShaderResourceView::Create(GDX11Context* context, ID3D11ShaderResourceView* srv, ID3D11Texture2D* tex)
	{
		return std::shared_ptr<ShaderResourceView>(new ShaderResourceView(context, srv, tex));
	}
}

