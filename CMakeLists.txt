# Configuration file to build cpico.

cmake_minimum_required(VERSION 2.8)
project(cpico)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

include_directories(AFTER SYSTEM include)
set(LIBRARY_FILES src/pico.c include/pico_errno.h include/pico_defs.h)
add_library(libcpico ${LIBRARY_FILES})
add_executable(cpico src/main.c)
target_link_libraries(cpico libcpico)

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
