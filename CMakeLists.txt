cmake_minimum_required(VERSION 3.6)
project(slax)

set(CMAKE_CXX_STANDARD 14)

include(conanbuildinfo.cmake)
CONAN_BASIC_SETUP()

set(SLAX_VERSION 1.0)
add_definitions(-DSLAX_VERSION=\"${SLAX_VERSION}\")

set(SOURCE_FILES main.cpp args.hxx)
add_executable(slax ${SOURCE_FILES})
target_link_libraries(slax ${CONAN_LIBS})
