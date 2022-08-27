#pragma once
#include <sstream>

#include "Event.h"
#include "../Core/MouseCodes.h"

namespace GDX11
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(const int x, const int y)
			: m_mouseX(x), m_mouseY(y) { }

		int GetMouseX() const { return m_mouseX; }
		int GetMouseY() const { return m_mouseY; }
		std::pair<int, int> GetMouse() const { return { m_mouseX, m_mouseY }; }

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		int m_mouseX, m_mouseY;
	};

	class MouseScrollEvent : public Event
	{
	public:
		MouseScrollEvent(const float axisX, const float axisY)
			: m_axisX(axisX), m_axisY(axisY)
		{
		}

		float GetAxisX() const { return m_axisX; }
		float GetAxisY() const { return m_axisY; }
		std::pair<float, float> GetAxis() const { return { m_axisX, m_axisY }; }

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_axisX, m_axisY;
	};

	class MouseButtonEvent : public Event
	{
	public:
		MouseButtonEvent(const MouseCode button)
			: m_button(button)
		{
		}

		MouseCode GetMouseButton() const { return m_button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryMouseButton | EventCategoryInput)

	protected:
		MouseCode m_button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		using MouseButtonEvent::MouseButtonEvent;

		virtual std::string ToString() const override
		{
			std::ostringstream oss;
			oss << "MouseButtonPressedEvent: " << m_button;
			return oss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		using MouseButtonEvent::MouseButtonEvent;

		virtual std::string ToString() const override
		{
			std::ostringstream oss;
			oss << "MouseButtonReleasedEvent: " << m_button;
			return oss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}