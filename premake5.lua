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

--include "extern/glfw.lua"

project "Tremble"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    includedirs {
        "code",
        "extern/SDL/include",
        "extern/glm",
        "extern/stb",
        "extern/glad/include"
    }
    files {
        "code/**.h",
        "code/**.cpp",
        "extern/glad/src/glad.c"
    }
    libdirs {
		"extern/SDL/VisualC/x64/Release"
    }
    links {
        "SDL3"
    }
    postbuildcommands {
        "{COPY} extern/SDL/VisualC/x64/Release/SDL3.dll data"
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