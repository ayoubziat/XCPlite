cmake_minimum_required(VERSION 3.1.0)
project(xcpLite)

set (XCPLITE_NAME xcpLite)

set(CMAKE_C_COMPILER "g++")
set(CMAKE_CXX_COMPILER "g++")

set(SOURCES main.cpp ecupp.cpp ecu.c xcpAppl.c xcpLite.c xcpTl.c util.c A2L.c xcpSlave.c clock.c)
set(CMAKE_BUILD_TYPE Debug)

add_executable(xcpLite ${SOURCES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(xcpLite PRIVATE Threads::Threads)

set (INSTALL_LIB_DIR lib CACHE STRING "Installation directory for libraries")
set (INSTALL_BIN_DIR bin CACHE STRING "Installation directory for executables")
set (INSTALL_INCLUDE_DIR include CACHE STRING "Installation directory for header files")
set(INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

file (GLOB_RECURSE xcpLite_INCLUDE RELATIVE ${INCLUDE_PATH} "*.h"  )
list (SORT xcpLite_INCLUDE)
foreach ( file ${xcpLite_INCLUDE} )
    get_filename_component( dir ${file} DIRECTORY )
    install( FILES "${INCLUDE_PATH}/${file}" DESTINATION "${INSTALL_INCLUDE_DIR}/xcpLite/${dir}" COMPONENT dev)
endforeach()

install (
    TARGETS ${XCPLITE_NAME} 
    EXPORT xcpLiteTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT
    LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT
    COMPONENT dev
)