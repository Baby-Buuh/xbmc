set(SOURCES KeyboardEasterEgg.cpp
            KeymapActionMap.cpp
            XkbcommonKeymap.cpp
)

set(HEADERS IActionMap.h
            IKeyboardHandler.h
            KeyboardEasterEgg.h
            KeymapActionMap.h
            XkbcommonKeymap.h
)

core_add_library(input_keyboard)
