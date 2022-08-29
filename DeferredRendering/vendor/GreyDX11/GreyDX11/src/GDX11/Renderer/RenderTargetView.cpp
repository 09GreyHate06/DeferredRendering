#include "RenderTargetView.h"
#include "../Core/GDX11Assert.h"

namespace GDX11
{
	RenderTargetView::RenderTargetView(GDX11Context* context, const D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc, const std::shared_ptr<Texture2D>& tex)
		: RenderingResource(context), m_rtTexture(tex)
	{
		GDX11_CORE_ASSERT(m_rtTexture, "Texture2D is null");

		D3D11_TEXTURE2D_DESC texDesc = {};
		m_rtTexture->GetNative()->GetDesc(&texDesc);
		GDX11_CORE_ASSERT(texDesc.BindFlags & D3D11_BIND_RENDER_TARGET, "Bind Flags must include D3D11_BIND_RENDER_TARGET");

		HRESULT hr;
		GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateRenderTargetView(m_rtTexture->GetNative(), &rtvDesc, &m_rtv));
	}

	RenderTargetView::RenderTargetView(GDX11Context* context, ID3D11RenderTargetView* rtv, const std::shared_ptr<Texture2D>& tex)
		: RenderingResource(context), m_rtv(rtv), m_rtTexture(tex)
	{
		GDX11_CORE_ASSERT(m_rtv, "RenderTargetView is null");
		GDX11_CORE_ASSERT(m_rtTexture, "Texture2D is null");
	}

	void RenderTargetView::Clear(float r, float g, float b, float a)
	{
		float color[] = { r, g, b, a };
		m_context->GetDeviceContext()->ClearRenderTargetView(m_rtv.Get(), color);
	}

	void RenderTargetView::Bind(const DepthStencilView* ds) const
	{
		ID3D11DepthStencilView* dsv = ds ? ds->GetNative() : nullptr;
		m_context->GetDeviceContext()->OMSetRenderTargets(1, m_rtv.GetAddressOf(), dsv);
	}

	void RenderTargetView::Bind(uint32_t numViews, const std::shared_ptr<RenderTargetView>* rtvs, const DepthStencilView* ds)
	{
		ID3D11DepthStencilView* dsv = ds ? ds->GetNative() : nullptr;
		std::vector<ID3D11RenderTargetView*> rtvArr(numViews);
		for (int i = 0; i < numViews; i++)
			rtvArr[i] = rtvs[i]->GetNative();

		rtvs[0]->GetContext()->GetDeviceContext()->OMSetRenderTargets(numViews, rtvArr.data(), dsv);
	}


	void RenderTargetView::Bind(const RenderTargetViewArray& rtva, const DepthStencilView* ds)
	{
		ID3D11DepthStencilView* dsv = ds ? ds->GetNative() : nullptr;
		std::vector<ID3D11RenderTargetView*> rtvArr(rtva.size());
		for (int i = 0; i < rtva.size(); i++)
			rtvArr[i] = rtva[i]->GetNative();

		rtva[0]->GetContext()->GetDeviceContext()->OMSetRenderTargets(rtva.size(), rtvArr.data(), dsv);
	}

	std::shared_ptr<RenderTargetView> RenderTargetView::Create(GDX11Context* context, const D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc, const std::shared_ptr<Texture2D>& tex)
	{
		return std::shared_ptr<RenderTargetView>(new RenderTargetView(context, rtvDesc, tex));
	}
	std::shared_ptr<RenderTargetView> RenderTargetView::Create(GDX11Context* context, ID3D11RenderTargetView* rtv, const std::shared_ptr<Texture2D>& tex)
	{
		return std::shared_ptr<RenderTargetView>(new RenderTargetView(context, rtv, tex));
	}
}
