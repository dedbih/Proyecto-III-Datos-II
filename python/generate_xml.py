import os
import xml.etree.ElementTree as ET

def create_node_config(project_root, node_id, port):
    """Creates XML config for a Disk Node with absolute paths"""
    config = ET.Element("config")
    ET.SubElement(config, "ip").text = "0.0.0.0"
    ET.SubElement(config, "port").text = str(port)

    # define paths relative to root
    storage_path = os.path.abspath(os.path.join(project_root, "storage", f"node{node_id}"))
    ET.SubElement(config, "path").text = storage_path

    # create directories if they don't exist
    os.makedirs(storage_path, exist_ok=True)
    os.makedirs(os.path.join(project_root, "disk_config"), exist_ok=True)

    # write XML file
    xml_path = os.path.join(project_root, "disk_config", f"node{node_id}.xml")
    ET.ElementTree(config).write(xml_path, encoding="utf-8", xml_declaration=True)
    print(f" Created: {xml_path}")
    print(f"   Storage: {storage_path}")

if __name__ == "__main__":
    # get project root
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    # generate configs for all nodes
    for i in range(1, 5):
        create_node_config(project_root, i, 5000 + i)

    print("All configurations generated in Proyecto III/disk_config/")