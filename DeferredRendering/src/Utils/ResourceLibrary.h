#pragma once
#include <GDX11.h>
#include <unordered_map>

namespace DRUtils
{
#define LEAF_ELEMENTS \
	X(BlendState) \
	X(Buffer) \
	X(DepthStencilState) \
	X(DepthStencilView) \
	X(RasterizerState) \
	X(RenderTargetView) \
	X(RenderTargetViewArray) \
	X(SamplerState) \
	X(VertexShader) \
	X(GeometryShader) \
	X(PixelShader) \
	X(Texture2D) \
	X(InputLayout) \
	X(ShaderResourceView)


	class ResourceLibrary
	{
	public:
		template<typename T>
		void Add(const std::string& key, const std::shared_ptr<T>& res)
		{
			static_assert(false);
		}

		template<typename T>
		std::shared_ptr<T> Get(const std::string& key) const
		{
			static_assert(false);
		}

		template<typename T>
		void Remove(const std::string& key)
		{
			static_assert(false);
		}

		template<typename T>
		bool Exist(const std::string& key) const
		{
			static_assert(false);
		}

		template<typename T>
		void Clear()
		{
			static_assert()
		}

		void ClearAll()
		{
#define X(e) m_##e.clear();
			LEAF_ELEMENTS
#undef X
		}

#define X(e) \
	template<> \
	void Add<GDX11::e>(const std::string& key, const std::shared_ptr<GDX11::e>& res) \
	{ \
		GDX11_ASSERT(m_##e.find(key) == m_##e.end(), "Storing Duplicate"); \
		m_##e[key] = res; \
	} 

		LEAF_ELEMENTS
#undef X



#define X(e) \
	template<> \
	std::shared_ptr<GDX11::e> Get<GDX11::e>(const std::string& key) const \
	{ \
		GDX11_ASSERT(m_##e.find(key) != m_##e.end(), "Resource does not exists"); \
		return m_##e.at(key); \
	} 

			LEAF_ELEMENTS
#undef X



#define X(e) \
	template<> \
	void Remove<GDX11::e>(const std::string& key) \
	{ \
		GDX11_ASSERT(m_##e.find(key) != m_##e.end(), "Resource does not exists"); \
		m_##e.erase(key); \
	}

			LEAF_ELEMENTS
#undef X



#define X(e) \
	template<> \
	bool Exist<GDX11::e>(const std::string& key) const \
	{ \
		return m_##e.find(key) != m_##e.end(); \
	}

			LEAF_ELEMENTS
#undef X



#define X(e) \
	template<> \
	void Clear<GDX11::e>() \
	{ \
		m_##e.clear(); \
	} 

			LEAF_ELEMENTS
#undef X

	private:
#define X(e) std::unordered_map<std::string, std::shared_ptr<GDX11::e>> m_##e;
		LEAF_ELEMENTS
#undef X

	};

#undef LEAF_ELEMENTS
}