cmake_minimum_required(VERSION 3.5)

project(Game_personality_analysis VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для Windows ресурсов
if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "app_icon.rc")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Quick QuickWidgets)

set(TS_FILES Game_personality_analysis_ru_RU.ts)

set(PROJECT_SOURCES
        app_icon.rc
        main.cpp
        GUI/mainwindow.cpp
        GUI/mainwindow.h
        GUI/mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Game_personality_analysis
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Game_personality_analysis APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(Game_personality_analysis SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Game_personality_analysis
            ${PROJECT_SOURCES}
            app_setting.cpp app_setting.h app_utility.cpp app_utility.h version.h
            source.qrc
            game/1/game_1.h game/1/game_1.cpp game/1/game_1.ui
            game/2/game_2.h game/2/game_2.cpp game/2/game_2.ui
            game/3/game_3.h game/3/game_3.cpp game/3/game_3.ui
            version.h
            GUI/info_dialog/info_dialog.h GUI/info_dialog/info_dialog.cpp GUI/info_dialog/info_dialog.ui
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# Генерация version.h через generate_version.bat
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.h
    COMMAND cmd /c ${CMAKE_SOURCE_DIR}/generate_version.bat
    DEPENDS ${CMAKE_SOURCE_DIR}/generate_version.bat
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating version.h from git info"
)

target_include_directories(Game_personality_analysis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Game_personality_analysis PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickWidgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Game_personality_analysis)
endif()
set_target_properties(Game_personality_analysis PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Game_personality_analysis
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Game_personality_analysis)
endif()
