#pragma once
#include "RenderingResource.h"
#include "Shader.h"

namespace GDX11
{
	class InputLayout : public RenderingResource<ID3D11InputLayout>
	{
	public:
		virtual ~InputLayout() = default;

		void Bind() const;
		virtual ID3D11InputLayout* GetNative() const override { return m_inputLayout.Get(); }

		static std::shared_ptr<InputLayout> Create(GDX11Context* context, const D3D11_INPUT_ELEMENT_DESC* inputElements, uint32_t numElements, ID3DBlob* byteCode);
		static std::shared_ptr<InputLayout> Create(GDX11Context* context, const std::shared_ptr<VertexShader>& vs);
		static std::shared_ptr<InputLayout> Create(GDX11Context* context, ID3D11InputLayout* inputLayout);

	private:
		InputLayout(GDX11Context* context, const D3D11_INPUT_ELEMENT_DESC* inputElements, uint32_t numElements, ID3DBlob* byteCode);
		InputLayout(GDX11Context* context, const std::shared_ptr<VertexShader>& vs);
		InputLayout(GDX11Context* context, ID3D11InputLayout* inputLayout);

		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	};
}