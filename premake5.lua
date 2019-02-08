workspace "V3"
    architecture "x64"

    configurations
    {
        "debug",
        "release",
        "dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "submodules/imgui"

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
        "%{prj.name}/source/**.cpp"
    }

    includedirs
    {
        "vendor/include",
        "submodules/imgui"
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

		libdirs { "vendor/lib/win64", "C:/VulkanSDK/1.1.97.0/Source/lib" }

		links { "vulkan-1", "glfw3dll", "glew32d", "opengl32" }

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../vendor/lib/win64/")
        }
		
		defines "V3_WIN64"
		defines "V3_WIN64_DLL"

    filter "configurations:debug"
        defines "V3_DEBUG"
        symbols "On"
        optimize "Off"
    filter { "system:windows", "configurations:debug" }
        buildoptions "/MDd"

    filter "configurations:release"
        defines "V3_RELEASE"
        optimize "On"

    filter "configurations:dist"
        defines "V3_DIST"
        optimize "On"

project "Client"
    location "Client"
    kind "ConsoleApp"
    language "C++"

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
        "Engine/source",
        "vendor/include",
        "submodules/imgui"
    }

    links
    {
        "Engine",
        "tbb",
        "imgui"
    }

	postbuildcommands
	{
		("{COPY} resources ../bin/" .. outputdir .. "/Client/resources")
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
            ("{COPY} ../vendor/lib/lin64 ../bin/" .. outputdir .. "/Client/")
        }

        defines "V3_LIN64"

	filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "10.0.17763.0"
		
		flags { "StaticRuntime", "MultiProcessorCompile" }
		
		libdirs { "vendor/lib/win64", "C:/VulkanSDK/1.1.97.0/Source/lib" }

        links
        { "vulkan-1", "glfw3dll", "glew32d", "opengl32" }

        postbuildcommands
        {
            ("{COPY} ../vendor/lib/win64/*.dll ../bin/" .. outputdir .. "/Client/"),
			("{COPY} ../vendor/lib/win64/*.lib ../bin/" .. outputdir .. "/Client/")
        }
		
		defines "V3_WIN64"

    filter "configurations:debug"
        defines "V3_DEBUG"
        symbols "On"
        links { "tbb_debug" }
    filter { "system:windows", "configurations:debug" }
        buildoptions "/MDd"

    filter "configurations:release"
        defines "V3_RELEASE"
        optimize "On"

    filter "configurations:dist"
        defines "V3_DIST"
        optimize "On"