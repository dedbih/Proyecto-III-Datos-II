cmake_minimum_required(VERSION 3.20)
project(Proyecto_III)

set(CMAKE_CXX_STANDARD 20)

# Set output to main project directory
set(OUTPUT_HEADER ${CMAKE_SOURCE_DIR}/blocks.hpp)  # Now in main folder

# PDF to blocks conversion
add_custom_command(
        OUTPUT ${OUTPUT_HEADER}
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/pdf2block.py
        ${CMAKE_SOURCE_DIR}/python/pedefe.pdf
        ${OUTPUT_HEADER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}  # Run from project root
        COMMENT "Generating blocks.hpp in project root"
        VERBATIM
)

# Main executable
add_executable(Proyecto_III ${CMAKE_SOURCE_DIR}/cpp/raid_parity.cpp)
target_include_directories(Proyecto_III PRIVATE
        ${CMAKE_SOURCE_DIR}  # Now looks in main directory
)
add_dependencies(Proyecto_III ${OUTPUT_HEADER})