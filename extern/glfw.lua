project "GLFW"
    kind "StaticLib"
    language "C"
    targetdir ("bin/%{prj.name}")
    objdir ("bin/obj/%{prj.name}")

     -- Path to GLFW source files
     files {
        "glfw/src/**.h",
        "glfw/src/**.c"
    }

    -- Exclude specific files not needed for static library
    removefiles {
        "glfw/src/win32_*.c",
        "glfw/src/wgl_*.c",
        "glfw/src/egl_context.c",
        "glfw/src/osmesa_context.c"
    }

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"

        -- Windows-specific files
        files {
            "glfw/src/win32_*.c",
            "glfw/src/wgl_*.c",
            "glfw/src/egl_context.c",
            "glfw/src/osmesa_context.c"
        }

        defines { 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"