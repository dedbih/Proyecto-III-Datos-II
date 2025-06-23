import os
import sys
import xml.etree.ElementTree as ET
from flask import Flask, request, jsonify
import base64
from flask_cors import CORS

app = Flask(__name__) # Crea una aplicación Flask y habilita CORS para permitir peticiones cruzadas
CORS(app)  # el Disk Node acepta solicitudes desde cualquier origen
STORAGE = {} # Inicializa diccionario vacío para guardar bloques en memoria

def load_config(config_path): # Toma una ruta de archivo XML como entrada
    """Load configuration from XML"""
    abs_path = os.path.abspath(config_path)
    print(f"Loading config from: {abs_path}")

    if not os.path.exists(abs_path):
        raise FileNotFoundError(f"Config file not found: {abs_path}")

    tree = ET.parse(abs_path)
    root = tree.getroot()

    storage_path = root.find('path').text
    # Convierte la ruta de almacenamiento a absoluta si es relativa
    if not os.path.isabs(storage_path):
        storage_path = os.path.abspath(storage_path)

    return { # Extrae y retorna la configuración (IP, puerto y ruta de almacenamiento)
        'ip': root.find('ip').text,
        'port': int(root.find('port').text),
        'path': storage_path
    }

@app.route('/store', methods=['POST'])
def store_block():
    """Store a data block with the given ID"""
    try: # Valida que la petición sea JSON y tenga los campos 'id' y 'data'
        if not request.is_json:
            return jsonify({"error": "Request must be JSON"}), 400

        data = request.get_json()
        if not data or 'id' not in data or 'data' not in data:
            return jsonify({"error": "Missing 'id' or 'data' in request"}), 400

        block_id = data['id']
        block_data = data['data']

        # Verifica que los datos sean una lista de bytes válidos (números 0-255)
        if not isinstance(block_data, list):
            return jsonify({"error": "Data must be a list of bytes"}), 400

        try:
            # Convierte la lista a bytes y almacena
            byte_data = bytes(block_data)
        except (ValueError, TypeError):
            return jsonify({"error": "Invalid byte values in data"}), 400

        #2 modos de almacenamiento
        #Depende de si se asigna una ruta de disco válida o no
        # Almacena en memoria (STORAGE)
        STORAGE[block_id] = byte_data

        # Almacena en disco como .bin, si STORAGE_PATH está configurado
        storage_path = app.config.get('STORAGE_PATH', '')
        if storage_path:
            file_path = os.path.join(storage_path, f"{block_id}.bin")
            with open(file_path, 'wb') as f:
                f.write(byte_data)
        # Retorna éxito o error
        return jsonify({
            "status": "success",
            "id": block_id,
            "size": len(byte_data)
        }), 200

    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/retrieve/<block_id>', methods=['GET'])
def retrieve_block(block_id): # Busca un bloque por su ID
    """Retrieve a stored block by ID"""
    try:
        # Primero en memoria (STORAGE)
        if block_id in STORAGE:
            return jsonify({
                "id": block_id,
                "data": list(STORAGE[block_id])  # Convert bytes to list of ints
            }), 200

        # Luego en disco (si STORAGE_PATH existe)
        storage_path = app.config.get('STORAGE_PATH', '')
        if storage_path:
            file_path = os.path.join(storage_path, f"{block_id}.bin")
            if os.path.exists(file_path):
                with open(file_path, 'rb') as f:
                    data = f.read()
                return jsonify({
                    "id": block_id,
                    "data": list(data)  # Si lo encuentra, retorna los datos como lista de bytes en JSON
                }), 200

        return jsonify({"error": "Block not found"}), 404 # Si no, retorna error 404

    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/status', methods=['GET']) #  información básica del nodo
def status():
    """Health check endpoint"""
    return jsonify({
        "status": "running", #Estado "running"
        "storage_path": app.config.get('STORAGE_PATH', ''), # Ruta de almacenamiento configurada
        "blocks_stored": len(STORAGE) # Cantidad de bloques almacenados en memoria
    }), 200

if __name__ == '__main__':
    if len(sys.argv) != 2: # Verifica que se pase un argumento (ruta al XML de configuración)
        print("Usage: python disk_node.py <config.xml>")
        sys.exit(1)

    try:
        config = load_config(sys.argv[1])  #Carga la configuración
        app.config['STORAGE_PATH'] = config['path'] # Configura la ruta de almacenamiento en flask

        # Crea el directorio de almacenamiento si no existe
        os.makedirs(config['path'], exist_ok=True)

        # Muestra información de inicio (dirección, endpoints disponibles)
        print(f"Starting Disk Node at {config['ip']}:{config['port']}")
        print(f"Storage path: {config['path']}")
        print(f"Available endpoints:")
        print(f"  POST /store - Store a data block")
        print(f"  GET  /retrieve/<id> - Retrieve a block")
        print(f"  GET  /status - Health check")
        # Inicia el servidor Flask con los parámetros del XML
        app.run(host=config['ip'], port=config['port'], threaded=True)

    except Exception as e:
        print(f"ERROR: {str(e)}")
        input("Press Enter to exit...") # Pausa antes de cerrar (para ver errores en Windows)