
IncludeDir["stb_image"] = "GreyDX11/vendor/stb_image/"
IncludeDir["spdlog"]    = "GreyDX11/vendor/spdlog/include"

project "GreyDX11"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    defines
    {
        "GDX11_IMGUI_SUPPORT",
    }

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.inl",
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.spdlog}",
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "GDX11_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "GDX11_RELEASE"
        runtime "Release"
        optimize "on"