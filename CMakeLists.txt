cmake_minimum_required(VERSION 3.20)
project(Proyecto_III)

# Find Python
find_package(Python REQUIRED COMPONENTS Interpreter)
set(CMAKE_CXX_STANDARD 20)

# Configuration variables
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})
set(PYTHON_DIR "${PROJECT_ROOT}/python")
set(STORAGE_DIR "${PROJECT_ROOT}/storage")
set(CONFIG_DIR "${PROJECT_ROOT}/disk_config")

# File paths
set(PDF_INPUT "${PYTHON_DIR}/pedefe.pdf")
set(OUTPUT_HEADER "${PROJECT_ROOT}/blocks.hpp")
set(OUTPUT_JSON "${PROJECT_ROOT}/blocks.json")
set(RECONSTRUCTED_PDF "${PROJECT_ROOT}/reconstructed.pdf")

# 1. Generate Disk Node configs
add_custom_target(GenerateConfigs ALL
        COMMAND ${Python_EXECUTABLE} "${PYTHON_DIR}/generate_xml.py"
        WORKING_DIRECTORY ${PROJECT_ROOT}
        COMMENT "Generating Disk Node config XMLs"
        BYPRODUCTS
        "${CONFIG_DIR}/node1.xml"
        "${CONFIG_DIR}/node2.xml"
        "${CONFIG_DIR}/node3.xml"
        "${CONFIG_DIR}/node4.xml"
)

# 2. Generate blocks.hpp from PDF
add_custom_target(GenerateBlocks ALL
        COMMAND ${Python_EXECUTABLE} "${PYTHON_DIR}/pdf2block.py"
        "${PDF_INPUT}" "${OUTPUT_HEADER}"
        WORKING_DIRECTORY ${PROJECT_ROOT}
        COMMENT "Converting PDF to C++ blocks header"
        DEPENDS GenerateConfigs
        BYPRODUCTS ${OUTPUT_HEADER}
)

# 3. Main controller executable
add_executable(Proyecto_III
        cpp/controller_node.cpp
)

target_include_directories(Proyecto_III PRIVATE
        ${PROJECT_ROOT}/include
        ${PROJECT_ROOT}  # For blocks.hpp
)

# Windows-specific linking
if(WIN32)
    target_link_libraries(Proyecto_III PRIVATE ws2_32)
endif()

add_dependencies(Proyecto_III GenerateBlocks)

# 4. Create storage directories
add_custom_target(CreateStorage ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${STORAGE_DIR}/node1
        COMMAND ${CMAKE_COMMAND} -E make_directory ${STORAGE_DIR}/node2
        COMMAND ${CMAKE_COMMAND} -E make_directory ${STORAGE_DIR}/node3
        COMMAND ${CMAKE_COMMAND} -E make_directory ${STORAGE_DIR}/node4
        COMMENT "Creating storage directories"
)

# 5. Run Controller Node
add_custom_target(RunController ALL
        COMMAND ${CMAKE_COMMAND} -E chdir ${PROJECT_ROOT} $<TARGET_FILE:Proyecto_III>
        COMMENT "Running Controller Node"
        DEPENDS Proyecto_III CreateStorage
)

# 6. PDF Reconstruction (optional - for testing)
add_custom_target(TestReconstruction ALL
        COMMAND ${Python_EXECUTABLE} "${PYTHON_DIR}/block2pdf.py"
        "${OUTPUT_JSON}" "${RECONSTRUCTED_PDF}"
        WORKING_DIRECTORY ${PROJECT_ROOT}
        COMMENT "Testing PDF reconstruction from blocks"
        DEPENDS RunController
        VERBATIM
)

# Final dependency chain
add_dependencies(TestReconstruction RunController)