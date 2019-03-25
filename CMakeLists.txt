cmake_minimum_required(VERSION 3.12.0)
project(otj)

file(GLOB SOURCES "*.cpp")
add_executable(otj ${SOURCES})

target_link_libraries(otj pthread boost_system csound64)
target_compile_features(otj PRIVATE cxx_std_17)

if(MSVC)
  target_compile_options(otj PRIVATE /W4 /WX)
else(MSVC)
  target_compile_options(otj PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

