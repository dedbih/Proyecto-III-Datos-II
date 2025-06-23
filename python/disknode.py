import os
import sys
import xml.etree.ElementTree as ET
from flask import Flask, request, jsonify
import base64
from flask_cors import CORS

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes
STORAGE = {}

def load_config(config_path):
    """Load configuration from XML"""
    abs_path = os.path.abspath(config_path)
    print(f"Loading config from: {abs_path}")

    if not os.path.exists(abs_path):
        raise FileNotFoundError(f"Config file not found: {abs_path}")

    tree = ET.parse(abs_path)
    root = tree.getroot()

    storage_path = root.find('path').text
    if not os.path.isabs(storage_path):
        storage_path = os.path.abspath(storage_path)

    return {
        'ip': root.find('ip').text,
        'port': int(root.find('port').text),
        'path': storage_path
    }

@app.route('/store', methods=['POST'])
def store_block():
    """Store a data block with the given ID"""
    try:
        if not request.is_json:
            return jsonify({"error": "Request must be JSON"}), 400

        data = request.get_json()
        if not data or 'id' not in data or 'data' not in data:
            return jsonify({"error": "Missing 'id' or 'data' in request"}), 400

        block_id = data['id']
        block_data = data['data']

        # Validate data is a list of bytes (0-255)
        if not isinstance(block_data, list):
            return jsonify({"error": "Data must be a list of bytes"}), 400

        try:
            # Convert list of integers to bytes
            byte_data = bytes(block_data)
        except (ValueError, TypeError):
            return jsonify({"error": "Invalid byte values in data"}), 400

        # Store in memory (you could save to disk here)
        STORAGE[block_id] = byte_data

        # Optionally save to filesystem
        storage_path = app.config.get('STORAGE_PATH', '')
        if storage_path:
            file_path = os.path.join(storage_path, f"{block_id}.bin")
            with open(file_path, 'wb') as f:
                f.write(byte_data)

        return jsonify({
            "status": "success",
            "id": block_id,
            "size": len(byte_data)
        }), 200

    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/retrieve/<block_id>', methods=['GET'])
def retrieve_block(block_id):
    """Retrieve a stored block by ID"""
    try:
        # First check memory storage
        if block_id in STORAGE:
            return jsonify({
                "id": block_id,
                "data": list(STORAGE[block_id])  # Convert bytes to list of ints
            }), 200

        # If not in memory, check filesystem
        storage_path = app.config.get('STORAGE_PATH', '')
        if storage_path:
            file_path = os.path.join(storage_path, f"{block_id}.bin")
            if os.path.exists(file_path):
                with open(file_path, 'rb') as f:
                    data = f.read()
                return jsonify({
                    "id": block_id,
                    "data": list(data)  # Convert bytes to list of ints
                }), 200

        return jsonify({"error": "Block not found"}), 404

    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/status', methods=['GET'])
def status():
    """Health check endpoint"""
    return jsonify({
        "status": "running",
        "storage_path": app.config.get('STORAGE_PATH', ''),
        "blocks_stored": len(STORAGE)
    }), 200

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python disk_node.py <config.xml>")
        sys.exit(1)

    try:
        config = load_config(sys.argv[1])
        app.config['STORAGE_PATH'] = config['path']

        # Create storage directory if it doesn't exist
        os.makedirs(config['path'], exist_ok=True)

        print(f"Starting Disk Node at {config['ip']}:{config['port']}")
        print(f"Storage path: {config['path']}")
        print(f"Available endpoints:")
        print(f"  POST /store - Store a data block")
        print(f"  GET  /retrieve/<id> - Retrieve a block")
        print(f"  GET  /status - Health check")

        app.run(host=config['ip'], port=config['port'], threaded=True)

    except Exception as e:
        print(f"ERROR: {str(e)}")
        input("Press Enter to exit...")