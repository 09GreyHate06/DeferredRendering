#pragma once
#include "RenderingResource.h"

namespace GDX11
{
	class DepthStencilState : public RenderingResource<ID3D11DepthStencilState>
	{
	public:
		virtual ~DepthStencilState() = default;

		void Bind(uint32_t stencilRef);

		virtual ID3D11DepthStencilState* GetNative() const override { return m_dss.Get(); }

		static std::shared_ptr<DepthStencilState> Create(GDX11Context* context, const D3D11_DEPTH_STENCIL_DESC& desc);
		static std::shared_ptr<DepthStencilState> Create(GDX11Context* context, ID3D11DepthStencilState* dss);

	private:
		DepthStencilState(GDX11Context* context, const D3D11_DEPTH_STENCIL_DESC& desc);
		DepthStencilState(GDX11Context* context, ID3D11DepthStencilState* dss);

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dss;
	};
}