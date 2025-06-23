import os
import sys
import xml.etree.ElementTree as ET
from flask import Flask, request, jsonify

app = Flask(__name__)
STORAGE = {}

def load_config(config_path):
    """Carga configuración desde XML"""
    # Convert to absolute path
    abs_path = os.path.abspath(config_path)
    print(f"Loading config from: {abs_path}")

    if not os.path.exists(abs_path):
        raise FileNotFoundError(f"Config file not found: {abs_path}")

    tree = ET.parse(abs_path)
    root = tree.getroot()

    # Convert storage path to absolute path
    storage_path = root.find('path').text
    if not os.path.isabs(storage_path):
        storage_path = os.path.abspath(storage_path)

    return {
        'ip': root.find('ip').text,
        'port': int(root.find('port').text),
        'path': storage_path
    }

# ... rest of your disknode.py code remains the same ...

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python disk_node.py <config.xml>")
        sys.exit(1)

    try:
        config = load_config(sys.argv[1])
        app.config['STORAGE_PATH'] = config['path']

        # Crear directorio si no existe
        os.makedirs(config['path'], exist_ok=True)

        print(f"Starting Disk Node at {config['ip']}:{config['port']}")
        print(f"Storage path: {config['path']}")
        app.run(host=config['ip'], port=config['port'])
    except Exception as e:
        print(f"ERROR: {str(e)}")
        input("Press Enter to exit...")  # Keeps window open on error