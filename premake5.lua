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
include "submodules/glfw"

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

    filter "system:linux"
        libdirs { "vendor/lib/lin64" }
        linkoptions { "-Wl,-rpath=\\$$ORIGIN/lin64" }
        defines "VIPER_LIN64"

	filter "system:windows"
        systemversion "latest"
		flags { "MultiProcessorCompile" }
        disablewarnings { "4996" }
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
        "submodules/cereal/include",
        "submodules/glfw/include"
    }

    filter "system:linux"
        libdirs { "vendor/lib/lin64" }
        linkoptions { "-Wl,-rpath=\\$$ORIGIN/lin64" }
        defines "VIPER_LIN64"

	filter "system:windows"
        systemversion "latest"
		flags { "MultiProcessorCompile" }
        disablewarnings { "4996" }
        includedirs { "C:/VulkanSDK/1.1.114.0/Include" }
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
        "submodules/cereal/include",
        "submodules/glfw/include"
    }

    links
    {
        "Viper",
        "Server",
        "Client",
        "glfw"
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
		libdirs { "vendor/lib/win64", "C:/VulkanSDK/1.1.114.0/Lib" }
        includedirs { "C:/VulkanSDK/1.1.114.0/Include" }
        links { "vulkan-1", "ws2_32", "lua53" }
		defines "VIPER_WIN64"

    filter "configurations:debug"
        defines "VIPER_DEBUG"
        links { "shaderc_combined_debug" }
        symbols "on"

    filter "configurations:release"
        defines "VIPER_RELEASE"
        links { "shaderc_combined" }
        optimize "on"

    filter "configurations:dist"
        defines "VIPER_DIST"
        links { "shaderc_combined" }
        optimize "on"