#pragma once
#include "RenderingResource.h"

namespace GDX11
{
	class Texture2D : public RenderingResource<ID3D11Texture2D>
	{
	public:
		virtual ~Texture2D() = default;

		virtual ID3D11Texture2D* GetNative() const override { return m_texture.Get(); }

		static std::shared_ptr<Texture2D> Create(GDX11Context* context, const D3D11_TEXTURE2D_DESC& texDesc, const D3D11_SUBRESOURCE_DATA* srd);
		static std::shared_ptr<Texture2D> Create(GDX11Context* context, const D3D11_TEXTURE2D_DESC& texDesc, const void* data);
		static std::shared_ptr<Texture2D> Create(GDX11Context* context, ID3D11Texture2D* tex);

	private:
		Texture2D(GDX11Context* context, const D3D11_TEXTURE2D_DESC& texDesc, const D3D11_SUBRESOURCE_DATA* srd);
		Texture2D(GDX11Context* context, const D3D11_TEXTURE2D_DESC& texDesc, const void* data);
		Texture2D(GDX11Context* context, ID3D11Texture2D* tex);

		void Init(const D3D11_TEXTURE2D_DESC& texDesc, const D3D11_SUBRESOURCE_DATA* srd);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
	};
}