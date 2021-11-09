cmake_minimum_required(VERSION 3.1.0)
project(xcpLite)

set(XCPLITE_NAME xcpLite)
set(XCPLITE_SOURCES ${XCPLITE_NAME}_SOURCES)
set(XCPLITE_INCLUDE ${XCPLITE_NAME}_INCLUDE)

set(CMAKE_C_COMPILER "g++")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE Debug)

set(INSTALL_LIB_DIR lib CACHE STRING "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE STRING "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE STRING "Installation directory for header files")
set(INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB XCPLITE_SOURCES "*.cpp" "*.c")
list(SORT XCPLITE_SOURCES)
add_executable(${XCPLITE_NAME} ${XCPLITE_SOURCES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${XCPLITE_NAME} PRIVATE Threads::Threads)

add_library(${XCPLITE_NAME}-src-lib STATIC ${XCPLITE_SOURCES})
target_link_libraries (${XCPLITE_NAME}-src-lib PRIVATE ${Boost_LIBRARIES} ${USE_RT} ${DL_LIBRARY} ${DLT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OS_LIBS})

file(GLOB_RECURSE XCPLITE_INCLUDE RELATIVE ${INCLUDE_PATH} "*.h" "*.hpp")
list(SORT XCPLITE_INCLUDE)
foreach( file ${XCPLITE_INCLUDE} )
    get_filename_component( dir ${file} DIRECTORY )
    install( FILES "${INCLUDE_PATH}/${file}" DESTINATION "${INSTALL_INCLUDE_DIR}/xcpLite/${dir}" COMPONENT dev)
endforeach()

install(
    TARGETS ${XCPLITE_NAME} 
    EXPORT xcpLiteTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT
    LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT
    COMPONENT dev
)

install(
    TARGETS ${XCPLITE_NAME}-src-lib
    LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
)
