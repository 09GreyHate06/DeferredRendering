#pragma once
#include "RenderingResource.h"

namespace GDX11
{
	class SamplerState : public RenderingResource<ID3D11SamplerState>
	{
	public:
		virtual ~SamplerState() = default;

		void VSBind(uint32_t slot = 0) const;
		void PSBind(uint32_t slot = 0) const;

		virtual ID3D11SamplerState* GetNative() const override { return m_samplerState.Get(); }

		static std::shared_ptr<SamplerState> Create(GDX11Context* context, const D3D11_SAMPLER_DESC& samplerDesc);
		static std::shared_ptr<SamplerState> Create(GDX11Context* context, ID3D11SamplerState* samplerState);

	private:
		SamplerState(GDX11Context* context, const D3D11_SAMPLER_DESC& samplerDesc);
		SamplerState(GDX11Context* context, ID3D11SamplerState* samplerState);

		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState = nullptr;
	};
}