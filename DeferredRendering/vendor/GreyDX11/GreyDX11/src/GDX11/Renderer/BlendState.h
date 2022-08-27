#pragma once
#include "RenderingResource.h"

namespace GDX11
{
	class BlendState : public RenderingResource<ID3D11BlendState>
	{
	public:
		virtual ~BlendState() = default;

		void Bind(const float* blendFactor, uint32_t sampleMask) const;

		virtual ID3D11BlendState* GetNative() const override { return m_bs.Get(); }

		static std::shared_ptr<BlendState> Create(GDX11Context* context, const D3D11_BLEND_DESC& desc);

	private:
		BlendState(GDX11Context* context, const D3D11_BLEND_DESC& desc);

		Microsoft::WRL::ComPtr<ID3D11BlendState> m_bs;
	};
}