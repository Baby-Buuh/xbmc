set(SOURCES Mmap.cpp
            PosixInterfaceForCLog.cpp
            Shm.cpp)

set(HEADERS FileHandle.h
            Mmap.h
            PosixInterfaceForCLog.h
            Shm.h)

core_add_library(utils_posix)
