#pragma once
#include <DirectXMath.h>

namespace CBuf
{
	namespace PS::deferred_lighting
	{
		static constexpr uint32_t s_lightMaxCount = 32;

		struct SystemCBuf
		{
			struct DirectionalLight
			{
				DirectX::XMFLOAT3 direction = { DirectX::XMConvertToRadians(50.0f), DirectX::XMConvertToRadians(-30.0f), 0.0f };
				float p0;
				DirectX::XMFLOAT3 ambient = { 0.2f, 0.2f, 0.2f };
				float p1;
				DirectX::XMFLOAT3 diffuse = { 0.8f, 0.8f, 0.8f };
				float p2;
				DirectX::XMFLOAT3 specular = { 1.0f, 1.0f, 1.0f };
				float p3;


				// for shadow calculation
				//DirectX::XMFLOAT4X4 viewProjMatrix;

			} dirLights[s_lightMaxCount];

			struct PointLight
			{
				DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
				float constant = 1.0f;
				DirectX::XMFLOAT3 ambient = { 0.2f, 0.2f, 0.2f };
				float linear = 0.14f;
				DirectX::XMFLOAT3 diffuse = { 0.8f, 0.8f, 0.8f };
				float quadratic = 0.0007f;
				DirectX::XMFLOAT3 specular = { 1.0f, 1.0f, 1.0f };
				float p0;


				// for shadow calculation
				//float farPlane;
				//float nearPlane;
				//float p1;
				//float p2;
				//DirectX::XMFLOAT4X4 viewMatrix;

			} pointLights[s_lightMaxCount];

			struct SpotLight
			{
				DirectX::XMFLOAT3 direction = { 0.0f, DirectX::XMConvertToRadians(90.0f), 0.0f };
				float constant = 1.0f;
				DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
				float linear = 0.14f;

				DirectX::XMFLOAT3 ambient = { 0.2f, 0.2f, 0.2f };
				float quadratic = 0.0007f;
				DirectX::XMFLOAT3 diffuse = { 0.8f, 0.8f, 0.8f };
				float innerCutOffAngleCos = cosf(10.0f);
				DirectX::XMFLOAT3 specular = { 1.0f, 1.0f, 1.0f };
				float outerCutOffAngleCos = cosf(15.0f);


				// for shadow calculation
				//DirectX::XMFLOAT4X4 viewProjMatrix;

			} spotLights[s_lightMaxCount];

			DirectX::XMFLOAT3 viewPosition;
			float p0;

			uint32_t activeDirLights = 0;
			uint32_t activePointLights = 0;
			uint32_t activeSpotLights = 0;
			float p1;
		};
	}
}