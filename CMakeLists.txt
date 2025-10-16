# Crest - Modern C REST API Framework
# CMake Build Configuration

cmake_minimum_required(VERSION 3.15)
project(Crest VERSION 1.0.0 LANGUAGES C CXX)

# C/C++ Standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(CREST_BUILD_SHARED "Build shared library" ON)
option(CREST_BUILD_STATIC "Build static library" ON)
option(CREST_BUILD_EXAMPLES "Build example applications" ON)
option(CREST_BUILD_TESTS "Build tests" OFF)
option(CREST_ENABLE_DASHBOARD "Enable web dashboard" ON)

# Platform detection
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    set(PLATFORM_LIBS ws2_32)
elseif(UNIX)
    set(PLATFORM_LIBS pthread)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Source files
set(CREST_CORE_SOURCES
    src/core/app.c
    src/core/config.c
    src/core/console.c
    src/core/request.c
    src/core/response.c
    src/core/server.c
    src/core/router.c
    src/core/middleware.c
)

set(CREST_UTIL_SOURCES
    src/utils/json.c
    src/utils/logger.c
    src/utils/string_utils.c
)

set(CREST_DOCS_SOURCES
    src/docs/api_docs.c
    src/docs/openapi.c
)

set(CREST_ALL_SOURCES
    ${CREST_CORE_SOURCES}
    ${CREST_UTIL_SOURCES}
    ${CREST_DOCS_SOURCES}
)

# Shared library
if(CREST_BUILD_SHARED)
    add_library(crest SHARED ${CREST_ALL_SOURCES})
    target_compile_definitions(crest PRIVATE CREST_BUILD_SHARED)
    target_link_libraries(crest ${PLATFORM_LIBS})
    target_include_directories(crest PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    set_target_properties(crest PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER "include/crest/crest.h;include/crest/types.h;include/crest/json.h;include/crest/router.h;include/crest/middleware.h;include/crest/openapi.h"
    )
endif()

# Static library
if(CREST_BUILD_STATIC)
    add_library(crest_static STATIC ${CREST_ALL_SOURCES})
    target_compile_definitions(crest_static PRIVATE CREST_STATIC)
    target_link_libraries(crest_static ${PLATFORM_LIBS})
    target_include_directories(crest_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    set_target_properties(crest_static PROPERTIES
        OUTPUT_NAME crest
        PUBLIC_HEADER "include/crest/crest.h;include/crest/types.h;include/crest/json.h;include/crest/router.h;include/crest/middleware.h;include/crest/openapi.h"
    )
endif()

# Examples
if(CREST_BUILD_EXAMPLES)
    # Basic example
    add_executable(crest_basic_example examples/basic/main.c)
    if(CREST_BUILD_SHARED)
        target_link_libraries(crest_basic_example crest)
    else()
        target_link_libraries(crest_basic_example crest_static ${PLATFORM_LIBS})
    endif()
    
    # Advanced example
    add_executable(crest_advanced_example examples/advanced/advanced.c)
    if(CREST_BUILD_SHARED)
        target_link_libraries(crest_advanced_example crest)
    else()
        target_link_libraries(crest_advanced_example crest_static ${PLATFORM_LIBS})
    endif()
endif()

# Tests
if(CREST_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)

if(CREST_BUILD_SHARED)
    install(TARGETS crest
        EXPORT CrestTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/crest
    )
endif()

if(CREST_BUILD_STATIC)
    install(TARGETS crest_static
        EXPORT CrestTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/crest
    )
endif()

# Install headers
install(DIRECTORY include/crest
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Generate and install CMake config files
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CrestConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CrestConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Crest
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/CrestConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CrestConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CrestConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Crest
)

install(EXPORT CrestTargets
    FILE CrestTargets.cmake
    NAMESPACE Crest::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Crest
)

# Print configuration summary
message(STATUS "")
message(STATUS "Crest Configuration Summary")
message(STATUS "============================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build shared library: ${CREST_BUILD_SHARED}")
message(STATUS "Build static library: ${CREST_BUILD_STATIC}")
message(STATUS "Build examples: ${CREST_BUILD_EXAMPLES}")
message(STATUS "Build tests: ${CREST_BUILD_TESTS}")
message(STATUS "Enable dashboard: ${CREST_ENABLE_DASHBOARD}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
