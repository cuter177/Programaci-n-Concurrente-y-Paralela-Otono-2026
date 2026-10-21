cmake_minimum_required(VERSION 3.16)
project(problema4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE FUENTES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

set(_limpias "")
foreach(_f ${FUENTES})
    file(RELATIVE_PATH _rel "${CMAKE_BINARY_DIR}" "${_f}")
    string(SUBSTRING "${_rel}" 0 2 _dos)
    if(_dos STREQUAL "..")
        list(APPEND _limpias "${_f}")
    endif()
endforeach()
set(FUENTES ${_limpias})

add_executable(problema4 ${FUENTES})

target_include_directories(problema4 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(problema4 PRIVATE Threads::Threads)
