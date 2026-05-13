cmake_minimum_required(VERSION 3.19)
project(DungeonRealm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(DungeonRealm
    WIN32 MACOSX_BUNDLE

    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    gameview.h gameview.cpp
    level2view.h level2view.cpp
    witchscene.h witchscene.cpp
    level3view.h level3view.cpp
    level3view.h level3view.cpp
    player.h player.cpp
    character.h character.cpp
    enemy.h enemy.cpp
    level.h level.cpp
    game.h game.cpp
)

target_link_libraries(DungeonRealm
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)

include(GNUInstallDirs)

# install(TARGETS DungeonRealm
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
install(TARGETS DungeonRealm
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
