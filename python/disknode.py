import os
import sys
import xml.etree.ElementTree as ET
from flask import Flask, request, jsonify

app = Flask(__name__)
STORAGE = {}

def load_config(config_path):
    """Carga configuración desde XML"""
    tree = ET.parse(config_path)
    root = tree.getroot()
    return {
        'ip': root.find('ip').text,
        'port': int(root.find('port').text),
        'path': root.find('path').text
    }

@app.route('/store', methods=['POST'])
def store_block():
    """Almacena un bloque de datos"""
    data = request.json
    block_id = data['id']
    content = bytes(data['data'])

    # Guardar en sistema de archivos
    block_path = os.path.join(app.config['STORAGE_PATH'], block_id)
    with open(block_path, 'wb') as f:
        f.write(content)

    STORAGE[block_id] = content
    return jsonify({"status": "success", "block": block_id}), 200

@app.route('/retrieve/<block_id>', methods=['GET'])
def retrieve_block(block_id):
    """Recupera un bloque de datos"""
    block_path = os.path.join(app.config['STORAGE_PATH'], block_id)

    if not os.path.exists(block_path):
        return jsonify({"error": "Block not found"}), 404

    with open(block_path, 'rb') as f:
        content = list(f.read())

    return jsonify({
        "id": block_id,
        "data": content
    }), 200

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python disk_node.py <config.xml>")
        sys.exit(1)

    config = load_config(sys.argv[1])
    app.config['STORAGE_PATH'] = config['path']

    # Crear directorio si no existe
    os.makedirs(config['path'], exist_ok=True)

    print(f"Starting Disk Node at {config['ip']}:{config['port']}")
    app.run(host=config['ip'], port=config['port'])