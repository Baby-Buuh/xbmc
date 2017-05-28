set(SOURCES Connection.cpp
            GLContextEGL.cpp
            WinEventsWayland.cpp
            WinSystemWayland.cpp
            WinSystemWaylandGLContext.cpp)

set(HEADERS Connection.h
            GLContextEGL.h
            WinEventsWayland.h
            WinSystemWayland.h
            WinSystemWaylandGLContext.h)

core_add_library(windowing_WAYLAND)
