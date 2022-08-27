#pragma once
#include <memory>
#include <spdlog/spdlog.h>

namespace GDX11
{
	class Log
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_coreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_clientLogger; }

	private:
		Log() = default;

		static bool s_init;
		static std::shared_ptr<spdlog::logger> s_coreLogger;
		static std::shared_ptr<spdlog::logger> s_clientLogger;
	};
}

// Core log macros
#define GDX11_CORE_LOG_TRACE(...)      GDX11::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define GDX11_CORE_LOG_INFO(...)       GDX11::Log::GetCoreLogger()->info(__VA_ARGS__)
#define GDX11_CORE_LOG_WARN(...)       GDX11::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define GDX11_CORE_LOG_ERROR(...)      GDX11::Log::GetCoreLogger()->error(__VA_ARGS__)
#define GDX11_CORE_LOG_CRITICAL(...)   GDX11::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define GDX11_LOG_TRACE(...)           GDX11::Log::GetClientLogger()->trace(__VA_ARGS__)
#define GDX11_LOG_INFO(...)            GDX11::Log::GetClientLogger()->info(__VA_ARGS__)
#define GDX11_LOG_WARN(...)            GDX11::Log::GetClientLogger()->warn(__VA_ARGS__)
#define GDX11_LOG_ERROR(...)           GDX11::Log::GetClientLogger()->error(__VA_ARGS__)
#define GDX11_LOG_CRITICAL(...)        GDX11::Log::GetClientLogger()->critical(__VA_ARGS__)