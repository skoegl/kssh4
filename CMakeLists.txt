find_package(KDE4 REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -std=c++0x -Wall)

include(KDE4Defaults)

include(MacroLibrary)

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)
