#pragma once
#include "GDX11Context.h"
#include "GDX11/Core/GDX11Assert.h"

namespace GDX11
{
	template<class T>
	class RenderingResource
	{
	public:
		virtual ~RenderingResource() = default;

		virtual T* GetNative() const = 0;

		virtual GDX11Context* GetContext() const { return m_context; }

	protected:
		RenderingResource(GDX11Context* context)
			: m_context(context)
		{
			GDX11_CORE_ASSERT(m_context, "Context is null");
		}

		GDX11Context* m_context;
	};
}