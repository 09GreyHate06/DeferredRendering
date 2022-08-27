#pragma once
#include "Camera.h"
#include <GDX11.h>

namespace DRUtils
{
	class CameraController
	{
	public:
		CameraController(Camera* camera, float viewportWidth, float viewportHeight, float distance, const DirectX::XMFLOAT3& focalPoint);
		CameraController();
		~CameraController() = default;

		void SetContext(Camera* camera);

		void OnEvent(GDX11::Event& event);
		void OnUpdate(const GDX11::Window* window);

		void Reset() { m_reset = true; }
		void SetDistance(float distance);
		void SetFocalPoint(const DirectX::XMFLOAT3& focalPoint);
		void SetViewportSize(float width, float height);

	private:
		bool OnMouseScroll(GDX11::MouseScrollEvent& event);

		void MousePan(const DirectX::XMFLOAT2& delta);
		void MouseRotate(const DirectX::XMFLOAT2& delta);
		void MouseZoom(float delta);
		//void MouseFreeZoom() const;

		DirectX::XMFLOAT3 CalculatePosition() const;
		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

		bool m_reset = false;
		float m_viewportWidth;
		float m_viewportHeight;
		float m_distance;
		Camera* m_camera;

		DirectX::XMFLOAT3 m_focalPoint;
		DirectX::XMFLOAT2 m_initialMousePos;
	};
}