#pragma once
#include "RenderingResource.h"

namespace GDX11
{
	class RasterizerState : public RenderingResource<ID3D11RasterizerState>
	{
	public:
		virtual ~RasterizerState() = default;

		void Bind() const;
		virtual ID3D11RasterizerState* GetNative() const override { return m_rs.Get(); }

		static std::shared_ptr<RasterizerState> Create(GDX11Context* context, const D3D11_RASTERIZER_DESC& desc);
		static std::shared_ptr<RasterizerState> Create(GDX11Context* context, ID3D11RasterizerState* rs);

	private:
		RasterizerState(GDX11Context* context, const D3D11_RASTERIZER_DESC& desc);
		RasterizerState(GDX11Context* context, ID3D11RasterizerState* rs);

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rs;
	};
}