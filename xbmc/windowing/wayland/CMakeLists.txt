set(SOURCES GLContextEGL.cpp
            WinSystemWayland.cpp
            WinSystemWaylandGLContext.cpp)

set(HEADERS GLContextEGL.h
            WinSystemWayland.h
            WinSystemWaylandGLContext.h)

core_add_library(windowing_WAYLAND)
