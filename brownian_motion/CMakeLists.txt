cmake_minimum_required(VERSION 3.0)

project(AgentBasedModelling VERSION 0.1.0 LANGUAGES CXX)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(brownian_agent brownian_agent.cpp)
