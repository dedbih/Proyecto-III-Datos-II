#!/bin/bash

# Iniciar Disk Nodes
python disk_node.py disk_config/node1.xml &
python disk_node.py disk_config/node2.xml &
python disk_node.py disk_config/node3.xml &
python disk_node.py disk_config/node4.xml &

# Compilar y ejecutar Controller Node
mkdir -p build && cd build
cmake .. && make
./controller_node &

echo "Sistema iniciado. Accede en: http://localhost:8080"