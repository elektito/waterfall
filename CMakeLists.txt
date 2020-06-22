cmake_minimum_required(VERSION 3.17)

project(waterfall VERSION 0.1 LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

add_executable(wf
  wf.c
  libs/glad/src/glad.c
)

target_include_directories(wf PRIVATE libs/glad/include libs/)
target_link_libraries(wf PRIVATE SDL2 dl m)
