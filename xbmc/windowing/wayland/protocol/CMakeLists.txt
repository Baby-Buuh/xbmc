set(SOURCES Compositor.cpp
            Connection.cpp
            Registry.cpp
            Shell.cpp
            ShellSurface.cpp)

set(HEADERS Compositor.h
            Connection.h
            ProtocolBase.h
            Registry.h
            Shell.h
            ShellSurface.h
            Surface.h)

core_add_library(windowing_WAYLAND_protocol)
