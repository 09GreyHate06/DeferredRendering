#pragma once
#include <stdint.h>

namespace GDX11
{
	using MouseCode = uint16_t;
	namespace Mouse
	{
		enum : MouseCode
		{
			/// <summary>
			/// Left mouse button.
			/// </summary>
			LeftButton = 0x01,

			/// <summary>
			/// Right mouse button.
			/// </summary>
			RightButton = 0x02,

			/// <summary>
			/// Middle mouse button (three-button mouse).
			/// </summary>
			MiddleButton = 0x04,

			/// <summary>
			/// Windows 2000/XP: X1 mouse button.
			/// </summary>
			XButton1 = 0x05,

			/// <summary>
			/// Windows 2000/XP: X2 mouse button.
			/// </summary>
			XButton2 = 0x06,
		};
	}
}