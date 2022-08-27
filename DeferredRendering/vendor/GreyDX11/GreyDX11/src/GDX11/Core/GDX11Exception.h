#pragma once
#include <exception>
#include <string>
#include <sstream>

namespace GDX11
{
	class GDX11Exception : public std::exception
	{
	public:
		GDX11Exception(int line, const std::string& file)
			: m_line(line), m_file(file) { }

		virtual const char* what() const override
		{
			std::ostringstream oss;
			oss << GetType() << '\n'
				<< GetOriginString();

			m_whatBuffer = oss.str();

			return m_whatBuffer.c_str();
		}

		virtual const char* GetType() const { return "GDX11Exception"; }

		std::string GetOriginString() const
		{
			std::ostringstream oss;
			oss << "[File] " << m_file << '\n'
				<< "[Line] " << m_line;

			return oss.str();
		}

		int GetLine() const { return m_line; }

	protected:
		mutable std::string m_whatBuffer;

	private:
		int m_line;
		std::string m_file;
	};
}