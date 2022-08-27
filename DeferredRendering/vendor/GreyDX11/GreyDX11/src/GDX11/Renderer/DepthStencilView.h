#pragma once 
#include "RenderingResource.h"
#include "Texture2D.h"

namespace GDX11
{
	class DepthStencilView : public RenderingResource<ID3D11DepthStencilView>
	{
	public:
		virtual ~DepthStencilView() = default;

		void Clear(uint32_t clearFlags, float depth, uint8_t stencil);

		// Bind depth stencil view without render target view
		void Bind();

		virtual ID3D11DepthStencilView* GetNative() const override { return m_dsv.Get(); }
		const std::shared_ptr<Texture2D>& GetTexture2D() const { return m_dsTexture; }

		static std::shared_ptr<DepthStencilView> Create(GDX11Context* context, const D3D11_DEPTH_STENCIL_VIEW_DESC& dsvDesc, const std::shared_ptr<Texture2D>& tex);
		static std::shared_ptr<DepthStencilView> Create(GDX11Context* context, ID3D11DepthStencilView* dsv, const std::shared_ptr<Texture2D>& tex);

	private:
		DepthStencilView(GDX11Context* context, const D3D11_DEPTH_STENCIL_VIEW_DESC& dsvDesc, const std::shared_ptr<Texture2D>& tex);
		DepthStencilView(GDX11Context* context, ID3D11DepthStencilView* dsv, const std::shared_ptr<Texture2D>& tex);

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;
		std::shared_ptr<Texture2D> m_dsTexture;
	};
}