set(SOURCES Connection.cpp
            GLContextEGL.cpp
            SeatInputProcessor.cpp
            WinEventsWayland.cpp
            WinSystemWayland.cpp
            WinSystemWaylandGLContext.cpp)

set(HEADERS Connection.h
            GLContextEGL.h
            SeatInputProcessor.h
            WinEventsWayland.h
            WinSystemWayland.h
            WinSystemWaylandGLContext.h)

core_add_library(windowing_WAYLAND)
