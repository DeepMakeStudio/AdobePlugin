cmake_minimum_required(VERSION 3.16)

project(Prompt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

set(Headers
    prompt_dialog.h
)
set(Sources
    main.cpp
    prompt_dialog.cpp
)

qt_add_executable(appPrompt
    ${Sources}
    ${Headers}
)

qt_add_qml_module(appPrompt
    URI Prompt
    VERSION 1.0
    QML_FILES Main.qml
)

set_target_properties(appPrompt PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER plugin.deepmake.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appPrompt
    PRIVATE Qt6::Quick
)

if(APPLE)
    set(QMAKE_APPLE_DEVICE_ARCHS x86_64 arm64)
    set(CMAKE_INSTALL_PREFIX "/Applications")
else()
    set(CMAKE_INSTALL_PREFIX "C:/Program Files/DeepMake/Prompt")
endif()

install(TARGETS appPrompt
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    #LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    # DESTINATION ${CMAKE_INSTALL_BINDIR}
)
qt_generate_deploy_app_script(
    TARGET appPrompt
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
message(STATUS "CMAKE_INSTALL_BINDIR='${CMAKE_INSTALL_BINDIR}'")
message(STATUS "deploy_script='${deploy_script}'")

# To install (and bundle the Qt frameworks)
# Win
#" C:\Qt\Tools\CMake_64\bin\cmake.exe" --install C:/Users/<user>/Documents/studio/video-plugins/ui/build-prompt-Desktop_Qt_6_5_3_MSVC2019_64bit-Release
# MacOs
# cmake --install /Users/<user>/Documents/Codebase/studio/video-plugins/ui/build-prompt-Qt_6_5_3_for_macOS-Release

# install doesn't appear to bundle all of the qt frameworks.
# this seems to do the trick on mac os
# /Users/<user>/Qt/6.5.3/macos/bin/macdeployqt6 /Users/<user>/Documents/studio/video-plugins/ui/build-prompt-Qt_6_5_3_for_macOS-Release/appPrompt.app
