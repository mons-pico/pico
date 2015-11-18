# Configuration file to build cpico.

cmake_minimum_required(VERSION 2.8)
project(cpico)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_MACOSX_RPATH 1)

include_directories(AFTER SYSTEM include)
file(GLOB LIBRARY_FILES "src/*.c")
add_library(cpico_static STATIC ${LIBRARY_FILES})
add_library(cpico_shared SHARED ${LIBRARY_FILES})
set_property( TARGET cpico_static PROPERTY POSITION_INDEPENDENT_CODE 1 )
if( UNIX )
    set_target_properties(cpico_shared PROPERTIES OUTPUT_NAME cpico)
    set_target_properties(cpico_static PROPERTIES OUTPUT_NAME cpico)
endif ( UNIX )
add_executable(pico-bin src/main.c)
target_link_libraries(pico-bin cpico_static)
set_target_properties(pico-bin PROPERTIES OUTPUT_NAME pico)

# Examples.
add_custom_target(examples)
file(GLOB EXAMPLES "examples/*.c")
foreach (EXAMPLE ${EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME_WE)
    add_executable(${EXAMPLE_NAME} EXCLUDE_FROM_ALL ${EXAMPLE})
    target_link_libraries(${EXAMPLE_NAME} cpico_static)
    add_dependencies(examples ${EXAMPLE_NAME})
endforeach(EXAMPLE)

# Testing.  Every .c file in the tests folder is assumed to be a test.
enable_testing()
file(GLOB TESTS "tests/*.c")
foreach (TEST ${TESTS})
    get_filename_component(TEST_NAME ${TEST} NAME_WE)
    add_executable(${TEST_NAME} ${TEST})
    target_link_libraries(${TEST_NAME} cpico_static)
    add_test(${TEST_NAME} ${TEST_NAME})
endforeach(TEST)

# Generate API documentation using Doxygen.
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc ALL
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
