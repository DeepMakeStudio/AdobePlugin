include(CMakePrintHelpers)
cmake_minimum_required(VERSION "3.26.0")


if(APPLE)
  # LibCPR doesn't appear to build as a universal binary on MacOS so I had to disable it.
  # needs more investigation
  option(BUILD_UNIVERSAL "Build a Universal binary on macOS" OFF)
  if(BUILD_UNIVERSAL)
    # Build a Universal binary on macOS
    # This requires that the found Qt library is compiled as Universal binaries.
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
  else()
    # Build for the host architecture on macOS
    #set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "${CMAKE_HOST_SYSTEM_PROCESSOR}" CACHE STRING "" FORCE)
  endif()

  set(CMAKE_INSTALL_PREFIX CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
endif()

if(MSVC)
    SET(CMAKE_GENERATOR_PLATFORM x64)
endif()

project("AK_VIDEO_PLUGINS")

set(OFX_PLUGIN_TARGET "ofx.plugin")
set(AE_PLUGIN_TARGET "ae.plugin")
set(UNIT_TEST_TARGET "unit.tests")
set(PIPL_TARGET "ae.pipl.target")

set(AE_PLUGIN_NAME "DeepMake_AE")
set(OFX_PLUGIN_NAME "DeepMake")

# if(MSVC)
#     add_compile_options(/W4 /WX)
# else()
#     add_compile_options(-Wall -Wextra -Wpedantic -Werror)
# endif()

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(CXX_EXTENSIONS OFF)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
enable_testing()

function(CleanUpDirectories)

    file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/bin)

endfunction(CleanUpDirectories)

function(add_core_library target_name)
    target_include_directories(${target_name} PRIVATE core)
    target_link_libraries(${target_name} akcore)
endfunction()

function(setup_ofx_plugin)
    set(OFXHeaders 
        plugins/ofx/ofx_host_delegate.h
        plugins/ofx/ofx_video_image.h
)
    set(OFXSources 
        plugins/ofx/ofx_main.cpp
        plugins/ofx/ofx_host_delegate.cpp
        plugins/ofx/ofx_video_image.cpp
    )

    add_library(${OFX_PLUGIN_TARGET} MODULE ${OFXSources} ${OFXHeaders})
    add_core_library(${OFX_PLUGIN_TARGET})

    if(APPLE)
        set_target_properties(${OFX_PLUGIN_TARGET} PROPERTIES
            BUNDLE True
            PREFIX ""
            OUTPUT_NAME "DeepMake.ofx"
            SUFFIX ".ofx"
            MACOSX_BUNDLE_GUI_IDENTIFIER com.deepmake.${OFX_PLUGIN_TARGET}
            MACOSX_BUNDLE_BUNDLE_NAME ${OFX_PLUGIN_TARGET}
            MACOSX_BUNDLE_BUNDLE_VERSION "0.2"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.2"
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/plugins/ofx/CMakeAOFXPluginInfo.plist.in
        )
    endif(APPLE)
    if(WIN32)
        #set_target_properties(${AE_PLUGIN_TARGET} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        set_target_properties(${OFX_PLUGIN_TARGET} PROPERTIES OUTPUT_NAME ${OFX_PLUGIN_NAME})
        set_target_properties(${OFX_PLUGIN_TARGET} PROPERTIES SUFFIX ".ofx")
    
    endif(WIN32)

    target_include_directories(${OFX_PLUGIN_TARGET} PRIVATE external/ofx-sdk)
endfunction()

function(compile_ae_pipl_resource)
    set(RESOURCE_FILE plugins/ae/ai_renderer_filter_pipl.r)
    if(APPLE)
        set(RESOURCE_OUTPUT ${CMAKE_BINARY_DIR}/bin/${AE_PLUGIN_NAME}.rsrc)

        # Add a custom command to compile the resource file
        add_custom_command(
            OUTPUT ${RESOURCE_OUTPUT}
            COMMAND Rez -i ${PROJECT_SOURCE_DIR} -o ${RESOURCE_OUTPUT}  -useDF -arch arm64 -arch x86_64 ${RESOURCE_FILE}
            DEPENDS ${RESOURCE_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Compiling AE PIPL resource file"
        )

        # Add a custom target to ensure the resource file is compiled before building the main target
        add_custom_target(${PIPL_TARGET} DEPENDS ${RESOURCE_OUTPUT})

        # Specify the location where the resource file should be copied in the bundle
        set(RESOURCE_OUTPUT_DESTINATION ${CMAKE_BINARY_DIR}/bin/${AE_PLUGIN_NAME}.bundle/Contents/Resources/${AE_PLUGIN_NAME}.rsrc)

        # Add a command to copy the resource file to the bundle's Resources folder
        add_custom_command(
            TARGET ${PIPL_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${RESOURCE_OUTPUT} ${RESOURCE_OUTPUT_DESTINATION}
            DEPENDS ${RESOURCE_OUTPUT}
            COMMENT "Copying AE PIPL resource file to bundle"
        )
    endif(APPLE)

    if(WIN32)
        # On Windows compiling the pipl resource is a bit more complicated
        # and requires multiple steps
        # 1 - Compile pipl to a .rr file using cl
        # 2 - Compile the pipl using the .rr as source, outputs a .rrc file
        # 3 - Compile the .rrc to a .rc file using cl
        set(PIPL_COMPILER external/ae-sdk/Resources/PiPLtool.exe)
        set(PIPL_INCLUDES external/ae-sdk/Headers)
        set(RESOURCE_FILE_COMPILED ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/ai_renderer_filter_pipl.rr)
        set(PIPL_COMPILED ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/ai_renderer_filter_pipl.rrc)
        set(RC_FILE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/ai_renderer_filter_pipl.rc)
        
       add_custom_command(
            OUTPUT ${RESOURCE_FILE_COMPILED}
            COMMAND ${CMAKE_CXX_COMPILER} /I ${PIPL_INCLUDES} /I ${PROJECT_SOURCE_DIR} /EP ${RESOURCE_FILE} > ${RESOURCE_FILE_COMPILED}
            DEPENDS ${RESOURCE_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Compiling AE PIPL resource file with cl"
        ) 
        add_custom_command(
            OUTPUT ${PIPL_COMPILED}
            COMMAND ${PIPL_COMPILER} ${RESOURCE_FILE_COMPILED} ${PIPL_COMPILED}
            DEPENDS ${RESOURCE_FILE_COMPILED}  # Updated dependency here
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Compiling AE PIPL resource using AE's PiplTool"
        )
        add_custom_command(
            OUTPUT ${RC_FILE}
            COMMAND ${CMAKE_CXX_COMPILER} /D "MSWindows" /EP ${PIPL_COMPILED} > ${RC_FILE}
            DEPENDS ${PIPL_COMPILED}  # Updated dependency here
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Compiling AE PIPL resource file with cl"
        )
        add_custom_target(${PIPL_TARGET} DEPENDS ${RESOURCE_FILE_COMPILED} ${RC_FILE})

   
        
    endif(WIN32)

endfunction()

function(setup_ae_plugin)
    set(AEHeaders 
        core/plugins/plugin_defs.h
        plugins/ae/ae_main.h
        plugins/ae/ae_host_delegate.h
        plugins/ae/ae_draw_helper.h
        plugins/ae/ae_video_image.h
        plugins/ae/ae_video_image_base.h
        plugins/ae/ae_video_image_checkout.h
        external/ae-sdk/Util/AEFX_SuiteHelper.h
        external/ae-sdk/Util/AEGP_SuiteHandler.h
        external/ae-sdk/Util/AEGP_Utils.h
    )
    set(AESources 
        plugins/ae/ae_main.cpp
        plugins/ae/ae_host_delegate.cpp
        plugins/ae/ae_draw_helper.cpp
        plugins/ae/ae_video_image.cpp
        plugins/ae/ae_video_image_base.cpp
        plugins/ae/ae_video_image_checkout.cpp
        external/ae-sdk/Util/AEFX_SuiteHelper.c
        external/ae-sdk/Util/AEGP_SuiteHandler.cpp
        external/ae-sdk/Util/AEGP_Utils.cpp
        external/ae-sdk/Util/MissingSuiteError.cpp
    )
    if(WIN32)
        set(AEResources 
            ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/ai_renderer_filter_pipl.rc        
        )

    endif(WIN32)

    compile_ae_pipl_resource()

    add_library(${AE_PLUGIN_TARGET} MODULE ${AESources} ${AEResources})            
    add_dependencies(${AE_PLUGIN_TARGET} ${PIPL_TARGET})

    add_core_library(${AE_PLUGIN_TARGET})

    if(APPLE)
        set_target_properties(${AE_PLUGIN_TARGET} PROPERTIES
            BUNDLE True
            OUTPUT_NAME ${AE_PLUGIN_NAME}
            MACOSX_BUNDLE_GUI_IDENTIFIER com.deepmake.${AE_PLUGIN_TARGET}
            MACOSX_BUNDLE_BUNDLE_NAME ${AE_PLUGIN_TARGET}
            MACOSX_BUNDLE_BUNDLE_VERSION "0.2"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.2"
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/plugins/ae/CMakeAEPluginInfo.plist.in
        )
    endif(APPLE)

    if(WIN32)
        #set_target_properties(${AE_PLUGIN_TARGET} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        set_target_properties(${AE_PLUGIN_TARGET} PROPERTIES SUFFIX ".aex")
        set_target_properties(${AE_PLUGIN_TARGET} PROPERTIES OUTPUT_NAME ${AE_PLUGIN_NAME})
        

       
    endif(WIN32)

    target_include_directories(${AE_PLUGIN_TARGET} SYSTEM PRIVATE 
        external/ae-sdk/Headers
        external/ae-sdk/Headers/adobesdk
        external/ae-sdk/Headers/adobesdk/config
        external/ae-sdk/Headers/adobesdk/drawbotsuites
        external/ae-sdk/Headers/SP
        external/ae-sdk/Util)
       
        
       
endfunction()

function(setup_unit_tests)

    set(TestHeaders 
    )
    set(TestSources 
        tests/tests.cpp
        tests/json_tests.cpp
        tests/param_tests.cpp
        tests/image_tests.cpp
    )

    add_subdirectory(external/googletest)
    add_executable(${UNIT_TEST_TARGET} ${TestSources} ${TestHeaders})
    # add_core_library(${UNIT_TEST_TARGET})

    target_link_libraries(${UNIT_TEST_TARGET} PUBLIC 
        gtest_main 
        akcore
    )

    add_test(
        NAME ${UNIT_TEST_TARGET}
        COMMAND ${UNIT_TEST_TARGET}
    )



endfunction()


# Function to set up configuration files
function(setup_config_file)
   
    # Specify the JSON path
    if(WIN32)
    set(json_path $ENV{APPDATA}\\DeepMake\\)
    else()
    set(json_path "$ENV{HOME}/Library/Application Support/DeepMake")
    message(STATUS "json_path: ${json_path}")
    endif(WIN32)
    # Call the function to generate or edit the JSON file
    generate_or_edit_json(${json_path} "Config")  
endfunction(setup_config_file)


# Function to generate or edit a JSON file
function(generate_or_edit_json json_path target_name)

    # Get the binary directory (where CMake is building)
    set(BINARY_DIR ${CMAKE_BINARY_DIR})

 # Go up two levels from the binary directory
    get_filename_component(UP_TWO_LEVELS ${BINARY_DIR} DIRECTORY)
    get_filename_component(UP_ONE_LEVEL ${UP_TWO_LEVELS} DIRECTORY)

# Specify the folder "fastapi_launcher"
    set(FASTAPI_LAUNCHER_DIR ${UP_ONE_LEVEL}/fastapi_launcher)

    
if(WIN32)

    # Check if the specified directory exists, and create it if not
    if(NOT EXISTS ${json_path})
            message(STATUS "Creating directory: ${json_path}")
            file(MAKE_DIRECTORY ${json_path})
    endif()
    set(USER_HOME "$ENV{USERPROFILE}")

    set(json_content
    "{ 
        \"Py_Environment\": \"${USER_HOME}\\.conda\\envs\\deepmake\\scripts\\uvicorn.exe\",
        \"Startup_CMD\": \" main:app\",
        \"Directory\": \"${FASTAPI_LAUNCHER_DIR}\"
    }"
    )

    # Replace single backslashes with triple backslashes
    string(REPLACE "\\" "\\\\" json_content "${json_content}")
    string(REPLACE "/" "\\\\" json_content "${json_content}")

    # Create or edit the JSON file with the specified target name
    file(WRITE ${json_path}/${target_name}.json ${json_content})
    file(WRITE ${BINARY_DIR}/${target_name}.json ${json_content})
    
    
endif(WIN32)

if(APPLE)   

set(USER_HOME "$ENV{HOME}")

set(json_content
"{ 
    \"Py_Environment\": \"conda activate deepmake &&\",
    \"Startup_CMD\": \" python startup.py\",
    \"Directory\": \"cd ${FASTAPI_LAUNCHER_DIR} &&\"
}"
)
 

if(NOT EXISTS ${json_path}/${target_name}.json )
    file(WRITE ${json_path}/${target_name}.json ${json_content})
endif()

if(NOT EXISTS ${BINARY_DIR}/${target_name}.json )
    file(WRITE ${BINARY_DIR}/${target_name}.json ${json_content})
endif()


endif(APPLE) 


    

message(STATUS "JSON file '${target_name}.json' created or updated in ${json_path}.")
endfunction(generate_or_edit_json)

# Call the setup_config_file function

function(CreateBinaryDirectories)
    
if (WIN32)
# After Effects
    set(WIN_AE_BUNDLE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/AfterEffects)
    file(MAKE_DIRECTORY ${WIN_AE_BUNDLE})
    file(MAKE_DIRECTORY $ENV{ProgramFiles}/DeepMake/)
    # Copy binary file to AE Folder
    add_custom_command(
        TARGET ${AE_PLUGIN_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${AE_PLUGIN_TARGET}> ${WIN_AE_BUNDLE}
    )

  install(PROGRAMS ${CMAKE_SOURCE_DIR}/ui/prompt-win-native/bin/appPrompt.exe DESTINATION $ENV{ProgramFiles}/DeepMake/)
  install(FILES ${WIN_AE_BUNDLE}/${AE_PLUGIN_NAME}.aex  DESTINATION "$ENV{ProgramFiles}/Adobe/Common/Plug-ins/7.0/MediaCore/" )
  message(STATUS "Adding Target Destination and file ${WIN_AE_BUNDLE}/${AE_PLUGIN_NAME}.aex, to install path tree")

  if(EXISTS ${WIN_AE_BUNDLE}/${AE_PLUGIN_NAME}.aex)
    add_custom_command(
        TARGET ${AE_PLUGIN_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -P cmake_install.cmake
    )
else()
   message(STATUS "File ${WIN_AE_BUNDLE}/${AE_PLUGIN_NAME}.aex, does not exist")
endif()
#OFX
    set(WIN_OFX_BUNDLE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/OFX)
    file(MAKE_DIRECTORY ${WIN_OFX_BUNDLE})

    # Copy binary file to OFX Folder
    add_custom_command(
        TARGET ${OFX_PLUGIN_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${OFX_PLUGIN_TARGET}> ${WIN_OFX_BUNDLE}
    )    
endif(WIN32)

if(APPLE)
#AE
#set(Mac_AE_BUNDLE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${AE_PLUGIN_NAME})
#file(MAKE_DIRECTORY ${Mac_AE_BUNDLE})
# Copy binary file to AE Folder
# if(NOT EXISTS ${Mac_AE_BUNDLE}/${AE_PLUGIN_NAME}.bundle)
#     add_custom_command(
#         TARGET ${AE_PLUGIN_TARGET} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${AE_PLUGIN_TARGET}> ${Mac_AE_BUNDLE}/${AE_PLUGIN_NAME}.bundle
#         COMMENT "Copying ${AE_PLUGIN_NAME}.bundle to ${Mac_AE_BUNDLE}"
#     )
# endif()
#cmake_print_variables(Mac_AE_BUNDLE)



install(DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/${AE_PLUGIN_NAME}.bundle  DESTINATION "/Library/Application Support/Adobe/Common/Plug-ins/7.0/MediaCore/")
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/ui/prompt-mac-native/bin/appPrompt.app DESTINATION "/Applications/")
install(DIRECTORY ${CMAKE_SOURCE_DIR}/ui/prompt-mac-native/bin/appPrompt.app
        DESTINATION "/Applications/"
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
#install(PROGRAMS ${CMAKE_SOURCE_DIR}/ui/prompt-mac-native/bin/appPrompt.app DESTINATION "/Applications")
message(STATUS "Adding Target Destination and file ${Mac_AE_BUNDLE}/${AE_PLUGIN_NAME}.bundle, to install path tree")

InstallStep()

#OFX
set(MAC_OFX_BUNDLE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${OFX_PLUGIN_NAME}_OFX)
file(MAKE_DIRECTORY ${MAC_OFX_BUNDLE})

# Copy binary file to OFX Folder
add_custom_command(
    TARGET ${OFX_PLUGIN_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${OFX_PLUGIN_TARGET}> ${MAC_OFX_BUNDLE}/${OFX_PLUGIN_NAME}_OFX.bundle
)  
endif(APPLE)

if(LINUX)
# not sure where to install on linux yet
endif(LINUX)
    
endfunction(CreateBinaryDirectories)

function(InstallStep)
    #temporary file to store the output
set(OUTPUT_FILE ${CMAKE_BINARY_DIR}/user_prompt_output.txt)

# Add a custom command to run the AppleScript and capture the output to a file
add_custom_target( 
    RunUserScript
    COMMAND osascript ${CMAKE_SOURCE_DIR}/external/UserPrompt.scpt > ${OUTPUT_FILE}
    DEPENDS ${CMAKE_SOURCE_DIR}/external/UserPrompt.scpt 
)

add_dependencies(${AE_PLUGIN_TARGET} RunUserScript)

if(EXISTS ${OUTPUT_FILE})
    # Read the content of the output file
    file(STRINGS ${OUTPUT_FILE} Output)
    message(STATUS "Output: ${Output}")  
endif()

# Install file to AE plugins folder and delete the temporary file when done
if(EXISTS ${CMAKE_SOURCE_DIR}/build/bin/${AE_PLUGIN_NAME}.bundle)
    add_custom_command(  
        TARGET RunUserScript POST_BUILD
        COMMAND sudo -S <<< ${Output} ${CMAKE_COMMAND} -P cmake_install.cmake
        COMMAND ${CMAKE_COMMAND} -E remove ${OUTPUT_FILE}  # Remove the temporary output file
        COMMENT "Installing ${AE_PLUGIN_NAME}.bundle to /Library/Application Support/Adobe/Common/Plug-ins/7.0/MediaCore"
        DEPENDS ${OUTPUT_FILE}
    )
else()
    message(STATUS "File ${AE_PLUGIN_NAME}.bundle does not exist")
endif()
endfunction(InstallStep)

function(HandleInstallation)

    cmake_print_variables(MY_BUILD_TYPE)
    if(MY_BUILD_TYPE STREQUAL "InstallWithDebugInfo" OR MY_BUILD_TYPE STREQUAL "InstallRelease")
    CreateBinaryDirectories()
    endif()
endfunction(HandleInstallation)



add_subdirectory(core)

include_directories(.)
include_directories(core)
include_directories(core/filters)
include_directories(core/host)
include_directories(core/logging)
include_directories(core/parameters)
include_directories(core/main_api_connection)
include_directories(external/rapidjson/include)
setup_ofx_plugin()
setup_ae_plugin()
setup_unit_tests()
setup_config_file()
HandleInstallation()



