#include "Buffer.h"
#include "../Core/GDX11Assert.h"

namespace GDX11
{
	Buffer::Buffer(GDX11Context* context, const D3D11_BUFFER_DESC& desc, const void* data)
		: RenderingResource(context), m_desc(desc)
	{
		HRESULT hr;
		if (data)
		{
			D3D11_SUBRESOURCE_DATA srd = {};
			srd.pSysMem = data;
			GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateBuffer(&desc, &srd, &m_buffer));
		}
		else
		{
			GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateBuffer(&desc, nullptr, &m_buffer));
		}
	}

	void Buffer::BindAsVB() const
	{
		const uint32_t offset = 0;
		const uint32_t stride = m_desc.StructureByteStride;
		m_context->GetDeviceContext()->IASetVertexBuffers(0, 1, m_buffer.GetAddressOf(), &stride, &offset);
	}

	void Buffer::BindAsIB(DXGI_FORMAT format) const
	{
		m_context->GetDeviceContext()->IASetIndexBuffer(m_buffer.Get(), format, 0);
	}

	void Buffer::VSBindAsCBuf(uint32_t slot) const
	{
		m_context->GetDeviceContext()->VSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
	}

	void Buffer::GSBindAsCBuf(uint32_t slot) const
	{
		m_context->GetDeviceContext()->GSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
	}

	void Buffer::PSBindAsCBuf(uint32_t slot) const
	{
		m_context->GetDeviceContext()->PSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
	}

	void Buffer::SetData(const void* data)
	{
		HRESULT hr; 
		D3D11_MAPPED_SUBRESOURCE msr = {};
		GDX11_CONTEXT_THROW_INFO(m_context->GetDeviceContext()->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
		memcpy(msr.pData, data, GetDesc().ByteWidth);
		m_context->GetDeviceContext()->Unmap(m_buffer.Get(), 0);
	}

	std::shared_ptr<Buffer> Buffer::Create(GDX11Context* context, const D3D11_BUFFER_DESC& desc, const void* data)
	{
		return std::shared_ptr<Buffer>(new Buffer(context, desc, data));
	}
}
