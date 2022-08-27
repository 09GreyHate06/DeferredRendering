#include "InputLayout.h"

namespace GDX11
{
	InputLayout::InputLayout(GDX11Context* context, const D3D11_INPUT_ELEMENT_DESC* inputElements, uint32_t numElements, ID3DBlob* byteCode)
		: RenderingResource(context)
	{
		HRESULT hr;
		GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateInputLayout(inputElements, numElements,
			byteCode->GetBufferPointer(), byteCode->GetBufferSize(), &m_inputLayout));
	}

	InputLayout::InputLayout(GDX11Context* context, const std::shared_ptr<VertexShader>& vs)
		: RenderingResource(context)
	{
		auto reflection = vs->GetReflection();

		HRESULT hr;
		D3D11_SHADER_DESC shaderDesc = {};
		GDX11_CONTEXT_THROW_INFO(reflection->GetDesc(&shaderDesc));

		// Read input layout description from shader info
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
		for (int i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc = {};
			GDX11_CONTEXT_THROW_INFO(reflection->GetInputParameterDesc(i, &paramDesc));

			// fill out input element desc
			D3D11_INPUT_ELEMENT_DESC elementDesc = {};
			elementDesc.SemanticName = paramDesc.SemanticName;
			elementDesc.SemanticIndex = paramDesc.SemanticIndex;
			elementDesc.InputSlot = 0;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;

			// determine DXGI format
			if (paramDesc.Mask == 1)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if (paramDesc.Mask <= 3)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (paramDesc.Mask <= 7)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (paramDesc.Mask <= 15)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			inputLayoutDesc.push_back(elementDesc);
		}

		GDX11_CONTEXT_THROW_INFO(m_context->GetDevice()->CreateInputLayout(inputLayoutDesc.data(), static_cast<uint32_t>(inputLayoutDesc.size()),
			vs->GetByteCode()->GetBufferPointer(), vs->GetByteCode()->GetBufferSize(), &m_inputLayout));
	}

	InputLayout::InputLayout(GDX11Context* context, ID3D11InputLayout* inputLayout)
		: RenderingResource(context), m_inputLayout(inputLayout)
	{
		GDX11_CORE_ASSERT(m_inputLayout, "InputLayout is null");
	}

	void InputLayout::Bind() const
	{
		m_context->GetDeviceContext()->IASetInputLayout(m_inputLayout.Get());
	}

	std::shared_ptr<InputLayout> InputLayout::Create(GDX11Context* context, const D3D11_INPUT_ELEMENT_DESC* inputElements, uint32_t numElements, ID3DBlob* byteCode)
	{
		return std::shared_ptr<InputLayout>(new InputLayout(context, inputElements, numElements, byteCode));
	}

	std::shared_ptr<InputLayout> InputLayout::Create(GDX11Context* context, const std::shared_ptr<VertexShader>& vs)
	{
		return std::shared_ptr<InputLayout>(new InputLayout(context, vs));
	}

	std::shared_ptr<InputLayout> InputLayout::Create(GDX11Context* context, ID3D11InputLayout* inputLayout)
	{
		return std::shared_ptr<InputLayout>(new InputLayout(context, inputLayout));
	}
}