#include "Camera.h"

using namespace DirectX;

namespace DRUtils
{
	Camera::Camera(const CameraDesc& desc)
		: m_desc(desc)
	{
		XMStoreFloat4x4(&m_view, XMMatrixIdentity());
		XMStoreFloat4x4(&m_projection, XMMatrixIdentity());

		UpdateViewMatrix();
		UpdateProjectionMatrix();
	}

	Camera::Camera()
		: Camera(CameraDesc())
	{
	}

	void Camera::Set(const CameraDesc& desc)
	{
		m_desc = desc;

		UpdateViewMatrix();
		UpdateProjectionMatrix();
	}

	void Camera::SetPosition(const DirectX::XMFLOAT3& pos)
	{
		m_desc.position = pos;
		UpdateViewMatrix();
	}

	void Camera::SetFov(float fov)
	{
		m_desc.fov = fov;
		UpdateProjectionMatrix();
	}

	void Camera::SetAspect(float aspect)
	{
		m_desc.aspect = aspect;
		UpdateProjectionMatrix();
	}

	void Camera::SetNearPlane(float nearPlane)
	{
		m_desc.nearPlane = nearPlane;
		UpdateProjectionMatrix();
	}

	void Camera::SetFarPlane(float farPlane)
	{
		m_desc.farPlane = farPlane;
		UpdateProjectionMatrix();
	}

	void Camera::SetYaw(float yaw)
	{
		m_desc.yaw = yaw;
		UpdateViewMatrix();
	}

	void Camera::SetPitch(float pitch)
	{
		m_desc.pitch = pitch;
		UpdateViewMatrix();
	}

	DirectX::XMMATRIX Camera::GetViewMatrix() const
	{
		return XMLoadFloat4x4(&m_view);
	}

	DirectX::XMMATRIX Camera::GetProjectionMatrix() const
	{
		return XMLoadFloat4x4(&m_projection);
	}

	DirectX::XMVECTOR Camera::GetUpDirection() const
	{
		return XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), GetOrientation());
	}

	DirectX::XMVECTOR Camera::GetRightDirection() const
	{
		return XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), GetOrientation());
	}

	DirectX::XMVECTOR Camera::GetForwardDirection() const
	{
		return XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), GetOrientation());
	}

	DirectX::XMVECTOR Camera::GetOrientation() const
	{
		return XMQuaternionRotationRollPitchYaw(m_desc.pitch, m_desc.yaw, 0.0f);
	}

	void Camera::UpdateViewMatrix()
	{
		XMMATRIX view = XMMatrixIdentity();
		view = XMMatrixRotationQuaternion(GetOrientation()) * XMMatrixTranslation(m_desc.position.x, m_desc.position.y, m_desc.position.z);
		view = XMMatrixInverse(nullptr, view);
		XMStoreFloat4x4(&m_view, view);
	}

	void Camera::UpdateProjectionMatrix()
	{
		XMMATRIX projection = XMMatrixPerspectiveFovLH(m_desc.fov, m_desc.aspect, m_desc.nearPlane, m_desc.farPlane);
		XMStoreFloat4x4(&m_projection, projection);
	}

}