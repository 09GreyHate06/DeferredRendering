#pragma once
#include "RenderingResource.h"

namespace GDX11
{
	class Buffer : public RenderingResource<ID3D11Buffer>
	{
	public:
		virtual ~Buffer() = default;

		void BindAsVB() const;
		void BindAsIB(DXGI_FORMAT format) const;
		void VSBindAsCBuf(uint32_t slot) const;
		void GSBindAsCBuf(uint32_t slot) const;
		void PSBindAsCBuf(uint32_t slot) const;

		void SetData(const void* data);
		const D3D11_BUFFER_DESC& GetDesc() const
		{
			return m_desc;
		}

		virtual ID3D11Buffer* GetNative() const override { return m_buffer.Get(); }

		static std::shared_ptr<Buffer> Create(GDX11Context* context, const D3D11_BUFFER_DESC& desc, const void* data);

	private:
		Buffer(GDX11Context* context, const D3D11_BUFFER_DESC& desc, const void* data);
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

		// ID3D11Buffer::GetDesc(out) function doesn't fill out StructureByteStride (???)
		D3D11_BUFFER_DESC m_desc;
	};
}