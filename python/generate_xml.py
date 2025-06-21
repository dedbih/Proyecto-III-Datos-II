# generate_xmls.py
import os
import xml.etree.ElementTree as ET

def create_node_config(node_id, port, storage_path):
    """Crea un XML de configuración para un Disk Node."""
    config = ET.Element("config")
    ET.SubElement(config, "ip").text = "0.0.0.0"  # Escucha en todas las interfaces
    ET.SubElement(config, "port").text = str(port)
    ET.SubElement(config, "path").text = storage_path

    tree = ET.ElementTree(config)
    os.makedirs("disk_config", exist_ok=True)  # Asegura que la carpeta exista
    tree.write(f"disk_config/node{node_id}.xml", encoding="utf-8", xml_declaration=True)
    print(f"✅ Config creada: disk_config/node{node_id}.xml")

# Genera configs para 4 nodos (puertos 5001-5004)
if __name__ == "__main__":
    for i in range(1, 5):
        create_node_config(
            node_id=i,
            port=5000 + i,  # 5001, 5002, 5003, 5004
            storage_path=f"./storage/node{i}"  # Ruta relativa a la carpeta storage
        )
    print("🎉 ¡Todos los archivos XML se generaron en disk_config/!")