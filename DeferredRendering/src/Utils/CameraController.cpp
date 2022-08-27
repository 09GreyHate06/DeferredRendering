#include "CameraController.h"

using namespace GDX11;
using namespace DirectX;

namespace DRUtils
{
	CameraController::CameraController(Camera* camera, float viewportWidth, float viewportHeight, float distance, const DirectX::XMFLOAT3& focalPoint)
		: m_camera(camera), m_viewportWidth(viewportWidth), m_viewportHeight(viewportHeight), m_distance(distance), m_focalPoint(focalPoint), m_initialMousePos(0.0f, 0.0f)
	{
	}

	CameraController::CameraController()
		: CameraController(nullptr, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f })
	{
	}

	void CameraController::SetContext(Camera* camera)
	{
		m_camera = camera;
		m_camera->SetPosition(CalculatePosition());
	}

	void CameraController::OnEvent(GDX11::Event& event)
	{
		if (!m_camera) return;

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrollEvent>(GDX11_BIND_EVENT_FN(OnMouseScroll));
	}

	void CameraController::OnUpdate(const GDX11::Window* window)
	{
		if (!m_camera) return;

		if (m_reset)
		{
			m_initialMousePos = Input::GetMousePos(window);
			m_reset = false;
		}

		if (Input::GetKey(window, Key::LeftMenu))
		{
			XMVECTOR initialMousePosXM = XMLoadFloat2(&m_initialMousePos);
			XMVECTOR mousePosXM = XMLoadFloat2(&Input::GetMousePos(window));
			XMVECTOR deltaXM = (mousePosXM - initialMousePosXM) * 0.003f;
			XMStoreFloat2(&m_initialMousePos, mousePosXM);

			XMFLOAT2 delta;
			XMStoreFloat2(&delta, deltaXM);

			if (Input::GetKey(window, Key::LeftControl) && Input::GetMouseButton(window, Mouse::LeftButton))
				MousePan(delta);
			else if (Input::GetKey(window, Mouse::LeftButton))
				MouseRotate(delta);

			m_camera->SetPosition(CalculatePosition());
		}
	}

	void CameraController::SetDistance(float distance)
	{
		if (!m_camera) return;

		m_distance = distance;
		m_camera->SetPosition(CalculatePosition());
	}

	void CameraController::SetFocalPoint(const DirectX::XMFLOAT3& focalPoint)
	{
		if (!m_camera) return;

		m_focalPoint = focalPoint;
		m_camera->SetPosition(CalculatePosition());
	}

	void CameraController::SetViewportSize(float width, float height)
	{
		if (!m_camera) return;

		m_viewportWidth = width;
		m_viewportHeight = height;
		m_camera->SetAspect(width / height);
		m_camera->SetPosition(CalculatePosition());
	}

	bool CameraController::OnMouseScroll(GDX11::MouseScrollEvent& event)
	{
		MouseZoom(event.GetAxisY() * 0.1f);

		return false;
	}

	void CameraController::MousePan(const DirectX::XMFLOAT2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		XMVECTOR focalPointXM = XMLoadFloat3(&m_focalPoint);
		focalPointXM += -m_camera->GetRightDirection() * delta.x * xSpeed * m_distance;
		focalPointXM += m_camera->GetUpDirection() * delta.y * ySpeed * m_distance;
		XMStoreFloat3(&m_focalPoint, focalPointXM);
	}

	void CameraController::MouseRotate(const DirectX::XMFLOAT2& delta)
	{
		auto pitch = m_camera->GetDesc().pitch;
		auto yaw = m_camera->GetDesc().yaw;

		float yawSign = XMVectorGetY(m_camera->GetUpDirection()) < 0.0f ? 1.0f : -1.0f;
		yaw -= yawSign * delta.x * RotationSpeed();
		pitch += delta.y * RotationSpeed();

		m_camera->SetYaw(yaw);
		m_camera->SetPitch(pitch);
	}

	void CameraController::MouseZoom(float delta)
	{
		m_distance -= delta * ZoomSpeed();
		if (m_distance < 1.0f)
		{
			XMVECTOR focalPointXM = XMLoadFloat3(&m_focalPoint);
			focalPointXM += m_camera->GetForwardDirection();
			XMStoreFloat3(&m_focalPoint, focalPointXM);
			m_distance = 1.0f;
		}

		m_camera->SetPosition(CalculatePosition());
	}

	//void CameraController::MouseFreeZoom() const
	//{
	//}

	DirectX::XMFLOAT3 CameraController::CalculatePosition() const
	{
		XMVECTOR focalPointXM = XMLoadFloat3(&m_focalPoint);
		XMVECTOR posXM = focalPointXM - m_camera->GetForwardDirection() * m_distance;

		XMFLOAT3 pos;
		XMStoreFloat3(&pos, posXM);
		return pos;
	}

	std::pair<float, float> CameraController::PanSpeed() const
	{
		float x = std::min(m_viewportWidth / 1000.0f, 2.4f); // max 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_viewportHeight / 1000.0f, 2.4f); // max 2.4
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float CameraController::RotationSpeed() const
	{
		return 0.8f;
	}

	float CameraController::ZoomSpeed() const
	{
		float distance = m_distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

}