#pragma once
#include <DirectXMath.h>

namespace DRUtils
{
	struct CameraDesc
	{
		float fov = 45.0f, aspect = 1280.0f / 720.0f, nearPlane = 0.1f, farPlane = 1000.0f;
		float yaw = 0.0f, pitch = 0.0f;
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	};

	class Camera
	{
	public:
		Camera(const CameraDesc& desc);
		Camera();
		virtual ~Camera() = default;

		void Set(const CameraDesc& desc);

		void SetPosition(const DirectX::XMFLOAT3& pos);
		void SetFov(float fov);
		void SetAspect(float aspect);
		void SetNearPlane(float nearPlane);
		void SetFarPlane(float farPlane);
		void SetYaw(float yaw);
		void SetPitch(float pitch);

		const CameraDesc& GetDesc() const { return m_desc; }

		virtual DirectX::XMMATRIX GetViewMatrix() const;
		virtual DirectX::XMMATRIX GetProjectionMatrix() const;
		virtual DirectX::XMVECTOR GetUpDirection() const;
		virtual DirectX::XMVECTOR GetRightDirection() const;
		virtual DirectX::XMVECTOR GetForwardDirection() const;
		virtual DirectX::XMVECTOR GetOrientation() const;

	protected:
		void UpdateViewMatrix();
		void UpdateProjectionMatrix();

		CameraDesc m_desc;
		DirectX::XMFLOAT4X4 m_view;
		DirectX::XMFLOAT4X4 m_projection;
	};
}