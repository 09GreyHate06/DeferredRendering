#include "SamplerState.h"
#include "../Core/GDX11Assert.h"

namespace GDX11
{
	SamplerState::SamplerState(GDX11Context* context, const D3D11_SAMPLER_DESC& samplerDesc)
		: RenderingResource(context)
	{
		HRESULT hr;
		GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateSamplerState(&samplerDesc, &m_samplerState));
	}

	SamplerState::SamplerState(GDX11Context* context, ID3D11SamplerState* samplerState)
		: RenderingResource(context), m_samplerState(samplerState)
	{
		GDX11_CORE_ASSERT(m_samplerState, "SamplerState is null");
	}

	void SamplerState::VSBind(uint32_t slot) const
	{
		m_context->GetDeviceContext()->VSSetSamplers(slot, 1, m_samplerState.GetAddressOf());
	}

	void SamplerState::PSBind(uint32_t slot) const
	{
		m_context->GetDeviceContext()->PSSetSamplers(slot, 1, m_samplerState.GetAddressOf());
	}

	std::shared_ptr<SamplerState> SamplerState::Create(GDX11Context* context, const D3D11_SAMPLER_DESC& samplerDesc)
	{
		return std::shared_ptr<SamplerState>(new SamplerState(context, samplerDesc));
	}
	std::shared_ptr<SamplerState> SamplerState::Create(GDX11Context* context, ID3D11SamplerState* samplerState)
	{
		return std::shared_ptr<SamplerState>(new SamplerState(context, samplerState));
	}
}

