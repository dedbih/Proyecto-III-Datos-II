cmake_minimum_required(VERSION 3.20)
project(Proyecto_III)

# Find Python interpreter
find_package(Python REQUIRED COMPONENTS Interpreter)

set(CMAKE_CXX_STANDARD 20)

# Set paths
set(PDF_INPUT "${CMAKE_SOURCE_DIR}/python/pedefe.pdf")
set(OUTPUT_HEADER "${CMAKE_BINARY_DIR}/blocks.hpp")
set(OUTPUT_JSON "${CMAKE_BINARY_DIR}/blocks.json")
set(OUTPUT_PDF "${CMAKE_SOURCE_DIR}/reconstructed.pdf")

# Custom target for header generation
add_custom_target(
        GenerateBlocksHpp ALL
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/pdf2block.py
        ${PDF_INPUT} ${OUTPUT_HEADER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating blocks.hpp from PDF"
        BYPRODUCTS ${OUTPUT_HEADER}
)

# Main executable
add_executable(Proyecto_III ${CMAKE_SOURCE_DIR}/cpp/raid_parity.cpp)
target_include_directories(Proyecto_III PRIVATE ${CMAKE_BINARY_DIR})

# Make sure header is generated before compiling
add_dependencies(Proyecto_III GenerateBlocksHpp)

# Target to run the executable
add_custom_target(
        RunRaid ALL
        COMMAND $<TARGET_FILE:Proyecto_III>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running RAID simulation"
        DEPENDS Proyecto_III
)

# Target to reconstruct PDF
add_custom_target(
        ReconstructPDF ALL
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/block2pdf.py
        ${OUTPUT_JSON} ${OUTPUT_PDF}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Reconstructing PDF from blocks.json"
        DEPENDS RunRaid
)

# Make all targets build in correct order
add_dependencies(RunRaid Proyecto_III)
add_dependencies(ReconstructPDF RunRaid)