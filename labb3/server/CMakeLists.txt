cmake_minimum_required(VERSION 3.00)
project(project_name C)

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK REQUIRED gtk+-3.0)
INCLUDE_DIRECTORIES(${GTK_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK_LIBRARY_DIRS})
ADD_DEFINITIONS(${GTK_CFLAGS_OTHER})

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lrt -lm")

include_directories(HEADER_FILES)
file(GLOB MAIN
        "SOURCE_FILES/*.c"
        "HEADER_FILES/*.h"
    )
    add_executable(executable ${MAIN})

TARGET_LINK_LIBRARIES(executable ${GTK_LIBRARIES})

