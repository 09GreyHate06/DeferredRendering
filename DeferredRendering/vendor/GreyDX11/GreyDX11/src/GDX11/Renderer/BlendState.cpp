#include "BlendState.h"

namespace GDX11
{
	BlendState::BlendState(GDX11Context* context, const D3D11_BLEND_DESC& desc)
		: RenderingResource(context)
	{
		HRESULT hr;
		GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateBlendState(&desc, &m_bs));
	}

	void BlendState::Bind(const float* blendFactor, uint32_t sampleMask) const
	{
		m_context->GetDeviceContext()->OMSetBlendState(m_bs.Get(), blendFactor, sampleMask);
	}

	std::shared_ptr<BlendState> BlendState::Create(GDX11Context* context, const D3D11_BLEND_DESC& desc)
	{
		return std::shared_ptr<BlendState>(new BlendState(context, desc));
	}

}

