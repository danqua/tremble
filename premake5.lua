workspace "Tremble"
    architecture "x86_64"
    flags {
        "MultiProcessorCompile"
    }
    configurations {
        "Debug",
        "Release"
    }
    debugdir "data"
    targetdir ("bin/%{prj.name}")
    objdir ("bin/obj/%{prj.name}")

include "extern/glfw.lua"

project "Tremble"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    includedirs {
        "code",
        "extern/glfw/include",
        "extern/glm",
        "extern/stb",
        "extern/glad/include"
    }
    files {
        "code/**.h",
        "code/**.cpp",
        "extern/glad/src/glad.c"
    }
    links {
        "GLFW"
    }

    disablewarnings {
		"4996"
	}
    filter "system:windows"
        systemversion "latest"
        staticruntime "On"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Full"