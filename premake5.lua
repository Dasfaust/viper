workspace "V3"
    architecture "x64"

    configurations
    {
        "debug",
        "release",
        "dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "submodules/MemoryPool"
include "submodules/concurrentqueue"
include "submodules/rapidjson"

project "Engine"
    location "Engine"
    kind "SharedLib"
    language "C++"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/source/**.h",
        "%{prj.name}/source/**.hpp",
        "%{prj.name}/source/**.c",
        "%{prj.name}/source/**.cpp",
		"%{prj.name}/source/**.tcc"
    }

    includedirs
    {
        "vendor/include",
		"submodules/MemoryPool/C-11",
		"submodules/concurrentqueue",
        "submodules/rapidjson/include"
    }

    filter "system:linux"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../vendor/lib/lin64/")
        }

        defines "V3_LIN64"

	filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"
		
		flags { "StaticRuntime", "MultiProcessorCompile" }

		libdirs { "vendor/lib/win64" }

		links { "ws2_32" }

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../vendor/lib/win64/")
        }
		
		defines "V3_WIN64"
		defines "V3_WIN64_DLL"
	filter { "system:windows", "configurations:debug" }
		buildoptions "/MDd"

    filter "configurations:debug"
        defines "V3_DEBUG"
        symbols "On"
        optimize "Off"

    filter "configurations:release"
        defines "V3_RELEASE"
        optimize "On"

    filter "configurations:dist"
        defines "V3_DIST"
        optimize "On"

project "Server"
    location "Server"
    kind "ConsoleApp"
    language "C++"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/source/**.h",
        "%{prj.name}/source/**.hpp",
        "%{prj.name}/source/**.c",
        "%{prj.name}/source/**.cpp",
		"%{prj.name}/source/**.tcc"
    }

    includedirs
    {
        "Engine/source",
        "vendor/include",
		"submodules/MemoryPool/C-11",
		"submodules/concurrentqueue",
        "submodules/rapidjson/include"
    }

    links
    {
        "Engine",
        "tbb"
    }

	postbuildcommands
	{
		("{COPY} resources ../bin/" .. outputdir .. "/Server/resources")
	}

    filter "system:linux"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        libdirs { "vendor/lib/lin64" }

        links
        {
			"vulkan",
			"glfw",
            "pthread",
            "X11",
            "GL",
            "png",
            "GLEW"
        }

        linkoptions { "-Wl,-rpath=\\$$ORIGIN/lin64" }

        postbuildcommands
        {
            ("{COPY} ../vendor/lib/lin64 ../bin/" .. outputdir .. "/Server/")
        }

        defines "V3_LIN64"

	filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "10.0.17763.0"
		
		flags { "StaticRuntime", "MultiProcessorCompile" }
		
		libdirs { "vendor/lib/win64" }

        links
        { "ws2_32" }

        postbuildcommands
        {
            ("{COPY} ../vendor/lib/win64/*.dll ../bin/" .. outputdir .. "/Server/"),
			("{COPY} ../vendor/lib/win64/*.lib ../bin/" .. outputdir .. "/Server/")
        }
		
		defines "V3_WIN64"
	filter { "system:windows", "configurations:debug" }
        buildoptions "/MDd"

    filter "configurations:debug"
        defines "V3_DEBUG"
        symbols "On"
        links { "tbb_debug" }

    filter "configurations:release"
        defines "V3_RELEASE"
        optimize "On"

    filter "configurations:dist"
        defines "V3_DIST"
        optimize "On"