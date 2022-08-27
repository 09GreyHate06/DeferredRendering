#pragma once
#include "RenderingResource.h"
#include "Texture2D.h"

namespace GDX11
{
	class ShaderResourceView : public RenderingResource<ID3D11ShaderResourceView>
	{
	public:
		virtual ~ShaderResourceView() = default;

		void VSBind(uint32_t slot) const;
		void GSBind(uint32_t slot) const;
		void PSBind(uint32_t slot) const;

		const std::shared_ptr<Texture2D>& GetTexture2D() const { return m_texture; }
		virtual ID3D11ShaderResourceView* GetNative() const override { return m_srv.Get(); }

		static std::shared_ptr<ShaderResourceView> Create(GDX11Context* context, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, const std::shared_ptr<Texture2D>& tex);
		static std::shared_ptr<ShaderResourceView> Create(GDX11Context* context, ID3D11ShaderResourceView* srv, const std::shared_ptr<Texture2D>& tex);
		static std::shared_ptr<ShaderResourceView> Create(GDX11Context* context, ID3D11ShaderResourceView* srv, ID3D11Texture2D* tex);
		
	private:
		ShaderResourceView(GDX11Context* context, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, const std::shared_ptr<Texture2D>& tex);
		ShaderResourceView(GDX11Context* context, ID3D11ShaderResourceView* srv, const std::shared_ptr<Texture2D>& tex);
		ShaderResourceView(GDX11Context* context, ID3D11ShaderResourceView* srv, ID3D11Texture2D* tex);

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
		std::shared_ptr<Texture2D> m_texture;
	};
}