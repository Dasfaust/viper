workspace "Viper"
    architecture "x64"

    configurations
    {
        "debug",
        "release",
        "dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "submodules/concurrentqueue"
include "submodules/cereal"

project "Viper"
    location "Viper"
    kind "StaticLib"
    staticruntime "on"
    language "C++"
    cppdialect "C++17"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/source/**.h",
        "%{prj.name}/source/**.hpp",
        "%{prj.name}/source/**.c",
        "%{prj.name}/source/**.cpp"
    }

    includedirs
    {
        "vendor/include",
		"submodules/concurrentqueue",
        "submodules/cereal/include"
    }

    filter "system:linux"
        libdirs { "vendor/lib/lin64" }
        defines "VIPER_LIN64"

	filter "system:windows"
		systemversion "latest"
		flags { "MultiProcessorCompile" }
        disablewarnings { "4996" }
		libdirs { "vendor/lib/win64" }
		links { "lua53" }
		defines "VIPER_WIN64"

    filter "configurations:debug"
        defines "VIPER_DEBUG"
        symbols "on"
        optimize "off"

    filter "configurations:release"
        defines "VIPER_RELEASE"
        optimize "on"

    filter "configurations:dist"
        defines "VIPER_DIST"
        optimize "on"

project "Server"
    location "Server"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/source/**.h",
        "%{prj.name}/source/**.hpp",
        "%{prj.name}/source/**.c",
        "%{prj.name}/source/**.cpp"
    }

    includedirs
    {
        "Viper/source",
        "vendor/include",
		"submodules/concurrentqueue",
        "submodules/cereal/include"
    }

    links
    {
        "Viper"
    }

    filter "system:linux"
        libdirs { "vendor/lib/lin64" }
        linkoptions { "-Wl,-rpath=\\$$ORIGIN/lin64" }
        defines "VIPER_LIN64"

	filter "system:windows"
        systemversion "latest"
		flags { "MultiProcessorCompile" }
        disablewarnings { "4996" }
		libdirs { "vendor/lib/win64" }
        links { "ws2_32", "lua53" }
		defines "VIPER_WIN64"

    filter "configurations:debug"
        defines "VIPER_DEBUG"
        symbols "on"

    filter "configurations:release"
        defines "VIPER_RELEASE"
        optimize "on"

    filter "configurations:dist"
        defines "VIPER_DIST"
        optimize "on"

project "Client"
    location "Client"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/source/**.h",
        "%{prj.name}/source/**.hpp",
        "%{prj.name}/source/**.c",
        "%{prj.name}/source/**.cpp"
    }

    includedirs
    {
        "Viper/source",
        "Server/source",
        "vendor/include",
		"submodules/concurrentqueue",
        "submodules/cereal/include"
    }

    links
    {
        "Viper",
        "Server"
    }

    filter "system:linux"
        libdirs { "vendor/lib/lin64" }
        linkoptions { "-Wl,-rpath=\\$$ORIGIN/lin64" }
        defines "VIPER_LIN64"

	filter "system:windows"
        systemversion "latest"
		flags { "MultiProcessorCompile" }
        disablewarnings { "4996" }
		libdirs { "vendor/lib/win64" }
        links { "ws2_32", "lua53" }
		defines "VIPER_WIN64"

    filter "configurations:debug"
        defines "VIPER_DEBUG"
        symbols "on"

    filter "configurations:release"
        defines "VIPER_RELEASE"
        optimize "on"

    filter "configurations:dist"
        defines "VIPER_DIST"
        optimize "on"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/source/**.h",
        "%{prj.name}/source/**.hpp",
        "%{prj.name}/source/**.c",
        "%{prj.name}/source/**.cpp"
    }

    includedirs
    {
        "Viper/source",
        "Server/source",
        "Client/source",
        "vendor/include",
		"submodules/concurrentqueue",
        "submodules/cereal/include"
    }

    links
    {
        "Viper",
        "Server",
        "Client"
    }

	postbuildcommands
	{
		("{COPY} resources ../bin/" .. outputdir .. "/Sandbox/resources")
	}

    filter "system:linux"
        libdirs { "vendor/lib/lin64" }
        linkoptions { "-Wl,-rpath=\\$$ORIGIN/lin64" }
        defines "VIPER_LIN64"

	filter "system:windows"
        systemversion "latest"
		flags { "MultiProcessorCompile" }
        disablewarnings { "4996" }
		libdirs { "vendor/lib/win64" }
        links { "ws2_32", "lua53" }
		defines "VIPER_WIN64"

    filter "configurations:debug"
        defines "VIPER_DEBUG"
        symbols "on"

    filter "configurations:release"
        defines "VIPER_RELEASE"
        optimize "on"

    filter "configurations:dist"
        defines "VIPER_DIST"
        optimize "on"