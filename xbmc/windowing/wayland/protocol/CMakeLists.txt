set(SOURCES Compositor.cpp
            Connection.cpp
            Registry.cpp)

set(HEADERS Compositor.h
            Connection.h
            Registry.h
            Types.h)

core_add_library(windowing_WAYLAND_protocol)
