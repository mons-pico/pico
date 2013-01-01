# Build the Pico library.
#

# Verify the version of CMake required.
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

# Establish the project, and the current version numbers.
# TODO Find out how to get these either from the XML or into the XML.
PROJECT( pico )
SET( PICO_MAJOR 0 )
SET( PICO_MINOR 0 )
SET( PICO_VERSION "${PICO_MAJOR}.${PICO_MINOR}" )
SET( PICO_DIST_NAME "pico-${PICO_VERSION}" )

# Specify the list of source files to use to build the library here.  We could
# use a glob, but that's not recommended, since cmake won't detect (caching)
# when files are added or removed.
#FILE( GLOB lib_sources "*.cpp" )
SET( lib_sources
    # None at present!
)

# Specify the list of tests to build.
SET( tests
    # None at present!
)

# Specify the C++ and C example programs.
SET( cpp_examples
    # None at present!
)
SET( c_examples
    # None at present!
)

# Determine if we are building for debug, or release.
IF( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE "Release" )
ENDIF( NOT CMAKE_BUILD_TYPE )

# Set the compiler flags.
SET( CMAKE_CXX_FLAGS "-g -Wall" )

# Modify the include path so we can find out own include files.
INCLUDE_DIRECTORIES( AFTER SYSTEM . io )

# Build the core library.
add_library( pico SHARED ${lib_sources} )
target_link_libraries( pico crypto )

# Add a test target that just builds the tests.
ADD_CUSTOM_TARGET(
    tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test
)

# Add a test target that actually runs the tests.
ADD_CUSTOM_TARGET(
    check
    ./run_tests.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test
)
ADD_DEPENDENCIES(check tests)

# Build the tests.
FOREACH( test_exec ${tests} )
    #STRING( REGEX REPLACE "test/(.*)\\.cpp" "\\1" test_exec ${test} )
    ADD_EXECUTABLE( ${test_exec} EXCLUDE_FROM_ALL ${test_exec}.cpp )
    TARGET_LINK_LIBRARIES( ${test_exec} pico )
    ADD_DEPENDENCIES( tests ${test_exec} )
ENDFOREACH( test_exec )

# Add a test target that just builds the C examples.
ADD_CUSTOM_TARGET(
    c_examples
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/capi
)
# Add a test target that just builds the C++ examples.
ADD_CUSTOM_TARGET(
    cpp_examples
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/cppapi
)
# Add a target that builds all examples.  We call this "samples" because
# the current version of CMake gets confused if a target share its name
# with a folder... or so it would seem.
ADD_CUSTOM_TARGET( samples )
ADD_DEPENDENCIES( samples c_examples cpp_examples )

# Build the example programs.
FOREACH( c_example ${c_examples} )
    ADD_EXECUTABLE( ${c_example} EXCLUDE_FROM_ALL ${c_example}.c )
    TARGET_LINK_LIBRARIES( ${c_example} pico )
    ADD_DEPENDENCIES( c_examples ${c_example} )
ENDFOREACH( c_example )
FOREACH( cpp_example ${cpp_examples} )
    ADD_EXECUTABLE( ${cpp_example} EXCLUDE_FROM_ALL ${cpp_example}.cpp )
    TARGET_LINK_LIBRARIES( ${cpp_example} pico )
    ADD_DEPENDENCIES( cpp_examples ${cpp_example} )
ENDFOREACH( cpp_example )

# Add a documentation target.  First find doxygen.
FIND_PROGRAM( doxygenPath doxygen PATHS ENV PATH NO_DEFAULT_PATH )
IF( doxygenPath )
    SET( docPath "${CMAKE_CURRENT_BINARY_DIR}/${TRI_DIST_NAME}/doc/api" )
    MESSAGE( "Found Doxygen at ${doxygenPath}." )
    MESSAGE( "Writing API documentation to ${docPath}" )
  FILE( MAKE_DIRECTORY "${docPath}" )
  ADD_CUSTOM_TARGET(
      doc
      ${doxygenPath} ${CMAKE_CURRENT_SOURCE_DIR}/pico.doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
ELSE( doxygenPath )
    MESSAGE( "WARNING: Doxygen not found; not generating documentation." )
    ADD_CUSTOM_TARGET(
        doc
        echo "WARNING: Doxygen not found; not generating documentation." )
ENDIF( doxygenPath )
