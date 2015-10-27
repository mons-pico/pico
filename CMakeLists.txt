# Configuration file to build cpico.

cmake_minimum_required(VERSION 3.3)
project(cpico)
set(ADDITIONAL_LIBS "")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Need to link with the OpenSSL library.
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
list(APPEND ADDITIONAL_LIBS ${OPENSSL_LIBRARIES})
find_library(CRYPTO_LIBRARY NAMES crypto)
list(APPEND ADDITIONAL_LIBS ${CRYPTO_LIBRARY})

include_directories(AFTER SYSTEM include)
set(LIBRARY_FILES src/pico.c include/pico_errno.h include/pico_defs.h)
add_library(pico ${LIBRARY_FILES})
add_executable(pico-bin src/main.c)
target_link_libraries(pico-bin pico ${ADDITIONAL_LIBS})
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
