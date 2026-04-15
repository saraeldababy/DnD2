cmake_minimum_required(VERSION 3.19)
project(DungeonRealm LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(DungeonRealm
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    gameview.h gameview.cpp
    dungeonmap.h dungeonmap.cpp
    player.h player.cpp
    character.h character.cpp
    enemy.h enemy.cpp
    level.h level.cpp
    game.h game.cpp
)

target_link_libraries(DungeonRealm
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS DungeonRealm
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET DungeonRealm
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
