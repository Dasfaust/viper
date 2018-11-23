workspace "V3"
    architecture "x64"

    configurations
    {
        "debug",
        "release",
        "dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
tbb_dir = "/home/cody/v3/vendor/tbb2019_20181010oss_lin/tbb2019_20181010oss/lib/intel64/gcc4.7"

project "Engine"
    location "Engine"
    kind "StaticLib"
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

    filter "system:linux"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        includedirs
        {
            "/home/cody/v3/vendor/linux/tbb2019_20181010oss_lin/tbb2019_20181010oss/include",
            "/home/cody/v3/vendor/linux/vulkan_1.1.85.0/include",
            "/home/cody/v3/vendor/linux/boost_1_68_0",
            "/home/cody/v3/vendor/linux/glfw-3.2.1/include"
        }

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

project "Client"
    location "Client"
    kind "ConsoleApp"
    language "C++"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")

    links
    {
        "Engine"
    }

    files
    {
        "%{prj.name}/source/**.h",
        "%{prj.name}/source/**.hpp",
        "%{prj.name}/source/**.c",
        "%{prj.name}/source/**.cpp"
    }

    filter "system:linux"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        includedirs
        {
            "/home/cody/v3/vendor/linux/tbb2019_20181010oss_lin/tbb2019_20181010oss/include",
            "/home/cody/v3/vendor/linux/vulkan_1.1.85.0/include",
            "/home/cody/v3/vendor/linux/boost_1_68_0",
            "/home/cody/v3/vendor/linux/glfw-3.2.1/include",
            "Engine/source"
        }

        libdirs
        {
            "/home/cody/v3/vendor/linux/glfw-3.2.1/src",
            "/home/cody/v3/vendor/linux/vulkan_1.1.85.0/source/lib",
            "/home/cody/v3/vendor/linux/tbb2019_20181010oss_lin/tbb2019_20181010oss/lib/intel64/gcc4.7"
        }

        links
        {
            "dl",
            "GLU",
            "GL",
            "rt",
            "Xrandr",
            "Xxf86vm",
            "Xi",
            "Xinerama",
            "X11",
            "Xcursor",
            "vulkan",
            "glfw3",
            "tbb",
            "tbb_debug",
            "pthread"
        }

        postbuildcommands
        {
            --("{COPY} /home/cody/v3/vendor/linux/tbb2019_20181010oss_lin/tbb2019_20181010oss/lib/intel64/gcc4.7/libtbb.so.2 ../bin/" .. outputdir .. "/%{prj.name}"),
            --("{COPY} /home/cody/v3/vendor/linux/glfw-3.2.1/src/libglfw.so.3 ../bin/" .. outputdir .. "/%{prj.name}")
        }

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