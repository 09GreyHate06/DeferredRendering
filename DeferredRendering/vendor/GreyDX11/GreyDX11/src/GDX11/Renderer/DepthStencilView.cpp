#include "DepthStencilView.h"
#include "../Core/GDX11Assert.h"

namespace GDX11
{
    void DepthStencilView::Clear(uint32_t clearFlags, float depth, uint8_t stencil)
    {
        m_context->GetDeviceContext()->ClearDepthStencilView(m_dsv.Get(), clearFlags, depth, stencil);
    }

    void DepthStencilView::Bind()
    {
        m_context->GetDeviceContext()->OMSetRenderTargets(0, nullptr, m_dsv.Get());
    }

    std::shared_ptr<DepthStencilView> GDX11::DepthStencilView::Create(GDX11Context* context, const D3D11_DEPTH_STENCIL_VIEW_DESC& dsvDesc, const std::shared_ptr<Texture2D>& tex)
    {
        return std::shared_ptr<DepthStencilView>(new DepthStencilView(context, dsvDesc, tex));
    }

    std::shared_ptr<DepthStencilView> DepthStencilView::Create(GDX11Context* context, ID3D11DepthStencilView* dsv, const std::shared_ptr<Texture2D>& tex)
    {
        return std::shared_ptr<DepthStencilView>(new DepthStencilView(context, dsv, tex));
    }

    DepthStencilView::DepthStencilView(GDX11Context* context, const D3D11_DEPTH_STENCIL_VIEW_DESC& dsvDesc, const std::shared_ptr<Texture2D>& tex)
        : RenderingResource(context), m_dsTexture(tex)
    {
        GDX11_CORE_ASSERT(m_dsTexture, "Texture2D is null");

        D3D11_TEXTURE2D_DESC texDesc = {};
        m_dsTexture->GetNative()->GetDesc(&texDesc);
        GDX11_CORE_ASSERT(texDesc.BindFlags & D3D11_BIND_DEPTH_STENCIL, "Bind Flags must include D3D11_BIND_DEPTH_STENCIL");

        HRESULT hr;
        GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateDepthStencilView(m_dsTexture->GetNative(), &dsvDesc, &m_dsv));
    }

    DepthStencilView::DepthStencilView(GDX11Context* context, ID3D11DepthStencilView* dsv, const std::shared_ptr<Texture2D>& tex)
        : RenderingResource(context), m_dsv(dsv), m_dsTexture(tex)
    {
        GDX11_CORE_ASSERT(m_dsv, "DepthStencilView is null");
        GDX11_CORE_ASSERT(m_dsTexture, "Texture2D is null");
    }
}