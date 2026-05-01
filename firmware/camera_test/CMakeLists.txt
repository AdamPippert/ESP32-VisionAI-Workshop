cmake_minimum_required(VERSION 3.16)

# Load local credentials file if present (gitignored — never committed)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig.defaults.local")
    set(SDKCONFIG_DEFAULTS "sdkconfig.defaults;sdkconfig.defaults.local")
else()
    set(SDKCONFIG_DEFAULTS "sdkconfig.defaults")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(camera_test)
