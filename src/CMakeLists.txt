set(kssh_SRCS kssh.cpp main.cpp)


include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})


set(QT_USE_QTCORE True)

include(${QT_USE_FILE})

kde4_add_ui_files(kssh_SRCS ksshdialog.ui)
kde4_add_app_icon(kssh_SRCS hi*-app-kssh.png)
kde4_add_executable(kssh ${kssh_SRCS})

target_link_libraries(kssh ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS kssh ${INSTALL_TARGETS_DEFAULT_ARGS})

kde4_install_icons(${ICON_INSTALL_DIR})
