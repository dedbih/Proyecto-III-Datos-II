cmake_minimum_required(VERSION 3.20)
project(Proyecto_III)

# Find Python
find_package(Python REQUIRED COMPONENTS Interpreter)
set(CMAKE_CXX_STANDARD 20)

# Use main directory for all outputs
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}")
set(PDF_INPUT "${CMAKE_SOURCE_DIR}/python/pedefe.pdf")
set(OUTPUT_HEADER "${OUTPUT_DIR}/blocks.hpp")
set(OUTPUT_JSON "${CMAKE_BINARY_DIR}/blocks.json")
set(OUTPUT_PDF "${OUTPUT_DIR}/reconstructed.pdf")

# 1. FIRST - Generate Disk Node configs
add_custom_target(GenerateConfigs ALL
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/generate_xmls.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating Disk Node config XMLs"
        BYPRODUCTS
        ${CMAKE_SOURCE_DIR}/disk_config/node1.xml
        ${CMAKE_SOURCE_DIR}/disk_config/node2.xml
        ${CMAKE_SOURCE_DIR}/disk_config/node3.xml
        ${CMAKE_SOURCE_DIR}/disk_config/node4.xml
)

# 2. Generate blocks.hpp
add_custom_target(GenerateBlocksHpp ALL
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/pdf2block.py
        "${PDF_INPUT}" "${OUTPUT_HEADER}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating blocks.hpp from PDF"
        BYPRODUCTS ${OUTPUT_HEADER}
)
add_dependencies(GenerateBlocksHpp GenerateConfigs)  # Configs must exist first

# 3. Main executable
add_executable(Proyecto_III
        ${CMAKE_SOURCE_DIR}/cpp/raid_parity.cpp
        ${CMAKE_SOURCE_DIR}/cpp/controller_node.cpp
)
target_include_directories(Proyecto_III PRIVATE ${OUTPUT_DIR})
add_dependencies(Proyecto_III GenerateBlocksHpp)

# 4. Create storage directories
add_custom_target(CreateStorageDirs ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/storage/node1
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/storage/node2
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/storage/node3
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/storage/node4
        COMMENT "Creating storage directories"
)

# 5. Run RAID program
add_custom_target(RunRaid ALL
        COMMAND ${CMAKE_COMMAND} -E chdir ${OUTPUT_DIR} $<TARGET_FILE:Proyecto_III>
        COMMENT "Running RAID simulation"
        DEPENDS Proyecto_III CreateStorageDirs
)

# 6. Reconstruct PDF
add_custom_target(ReconstructPDF ALL
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/block2pdf.py
        "${OUTPUT_JSON}" "${OUTPUT_PDF}"
        WORKING_DIRECTORY ${OUTPUT_DIR}
        COMMENT "Reconstructing PDF from blocks.json"
        DEPENDS RunRaid
        VERBATIM
)

# Final dependencies
add_dependencies(RunRaid Proyecto_III)
add_dependencies(ReconstructPDF RunRaid)