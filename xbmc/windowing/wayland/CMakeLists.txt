set(SOURCES Connection.cpp
            GLContextEGL.cpp
            WinSystemWayland.cpp
            WinSystemWaylandGLContext.cpp)

set(HEADERS Connection.h
            GLContextEGL.h
            WinSystemWayland.h
            WinSystemWaylandGLContext.h)

core_add_library(windowing_WAYLAND)
