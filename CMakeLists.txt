cmake_minimum_required(VERSION 3.5)

project(JigsawPuzzle VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp

        # Core game logic
        core/jigsaw_types.h
        core/puzzle_game.h
        core/puzzle_game.cpp
        core/puzzle_grid.h
        core/puzzle_grid.cpp

        # Puzzle components
        components/puzzle_piece.h
        components/puzzle_piece.cpp
        components/puzzle_path.h
        components/puzzle_path.cpp
        components/custom_puzzle_path.h
        components/custom_puzzle_path.cpp

        # UI components
        ui/puzzle_button.h
        ui/puzzle_button.cpp
        ui/puzzle_label.h
        ui/puzzle_label.cpp
        ui/puzzle_slider.h
        ui/puzzle_slider.cpp
        ui/game_menu.h
        ui/game_menu.cpp
        ui/main_window.h
        ui/main_window.cpp
        ui/main_window.ui

        # Tools and editors
        tools/path_creator.h
        tools/path_creator.cpp
        tools/creator_canvas.h
        tools/creator_canvas.cpp
        tools/image_effects.h
        tools/image_effects.cpp

        # Resources
        resources/backgrounds.qrc

        README.md
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(JigsawPuzzle
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET JigsawPuzzle APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(JigsawPuzzle SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(JigsawPuzzle
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(JigsawPuzzle PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(JigsawPuzzle PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(JigsawPuzzle)
endif()
