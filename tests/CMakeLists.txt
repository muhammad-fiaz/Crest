# Crest Test Suite
# CMake configuration for unit and integration tests

cmake_minimum_required(VERSION 3.15)

# Test executables
add_executable(test_json test_json.c)
add_executable(test_router test_router.c)
add_executable(test_pattern_matching test_pattern_matching.c)
add_executable(test_crest test_crest.cpp)
add_executable(test_integration test_integration.c)
add_executable(test_duplicate_routes test_duplicate_routes.c)

# Link against Crest library
if(CREST_BUILD_SHARED)
    target_link_libraries(test_json crest)
    target_link_libraries(test_router crest)
    target_link_libraries(test_pattern_matching crest)
    target_link_libraries(test_crest crest)
    target_link_libraries(test_integration crest)
    target_link_libraries(test_duplicate_routes crest)
else()
    target_link_libraries(test_json crest_static ${PLATFORM_LIBS})
    target_link_libraries(test_router crest_static ${PLATFORM_LIBS})
    target_link_libraries(test_pattern_matching crest_static ${PLATFORM_LIBS})
    target_link_libraries(test_crest crest_static ${PLATFORM_LIBS})
    target_link_libraries(test_integration crest_static ${PLATFORM_LIBS})
    target_link_libraries(test_duplicate_routes crest_static ${PLATFORM_LIBS})
    
    # Define CREST_STATIC for static linking
    target_compile_definitions(test_json PRIVATE CREST_STATIC)
    target_compile_definitions(test_router PRIVATE CREST_STATIC)
    target_compile_definitions(test_pattern_matching PRIVATE CREST_STATIC)
    target_compile_definitions(test_crest PRIVATE CREST_STATIC)
    target_compile_definitions(test_integration PRIVATE CREST_STATIC)
    target_compile_definitions(test_duplicate_routes PRIVATE CREST_STATIC)
endif()

# Register tests with CTest
add_test(NAME JSON_Parser_Tests COMMAND test_json)
add_test(NAME Router_Tests COMMAND test_router)
add_test(NAME Pattern_Matching_Tests COMMAND test_pattern_matching)
add_test(NAME Crest_CPP_Tests COMMAND test_crest)
add_test(NAME Integration_Tests COMMAND test_integration)
add_test(NAME Duplicate_Routes_Tests COMMAND test_duplicate_routes)

# Set test properties
set_tests_properties(JSON_Parser_Tests PROPERTIES
    TIMEOUT 30
    LABELS "unit;json"
)

set_tests_properties(Router_Tests PROPERTIES
    TIMEOUT 30
    LABELS "unit;router"
)

set_tests_properties(Pattern_Matching_Tests PROPERTIES
    TIMEOUT 30
    LABELS "unit;router;pattern"
)

set_tests_properties(Crest_CPP_Tests PROPERTIES
    TIMEOUT 60
    LABELS "unit;cpp;integration"
)

set_tests_properties(Integration_Tests PROPERTIES
    TIMEOUT 60
    LABELS "integration"
)

# Copy test data files if any
# file(COPY test_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "Configured ${CMAKE_PROJECT_NAME} tests")
message(STATUS "  - test_json: JSON parser and serializer")
message(STATUS "  - test_router: Route registration and matching")
message(STATUS "  - test_pattern_matching: Pattern matching functionality")
message(STATUS "  - test_crest: C++ unit tests for core functionality")
message(STATUS "  - test_integration: Integration tests for full framework")
