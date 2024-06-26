cmake_minimum_required(VERSION 3.0)
project(TankGame)

find_package(SFML 2.6 COMPONENTS graphics REQUIRED)

add_executable(TankGame Source.cpp)

target_link_libraries(TankGame sfml-graphics)
