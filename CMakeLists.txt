# Configuration file to build cpico.

cmake_minimum_required(VERSION 2.8)
project(cpico)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_MACOSX_RPATH 1)

include_directories(AFTER SYSTEM include)
set(LIBRARY_FILES src/pico.c include/pico_errno.h include/pico_defs.h src/md5.c src/md5.h)
add_library(cpico_static STATIC ${LIBRARY_FILES})
add_library(cpico_shared SHARED ${LIBRARY_FILES})
set_property( TARGET cpico_static PROPERTY POSITION_INDEPENDENT_CODE 1 )
if( UNIX )
    set_target_properties(cpico_shared PROPERTIES OUTPUT_NAME cpico)
    set_target_properties(cpico_static PROPERTIES OUTPUT_NAME cpico)
endif ( UNIX )
add_executable(pico-bin src/main.c)
target_link_libraries(pico-bin cpico_static )
set_target_properties(pico-bin PROPERTIES OUTPUT_NAME pico)

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
