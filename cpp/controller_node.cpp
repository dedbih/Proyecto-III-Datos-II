#include <iostream>
#include <vector>
#include <unordered_map>
#include "httplib.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include "json.hpp"
#include "blocks.hpp"

namespace fs = std::filesystem;
using namespace httplib;
using json = nlohmann::json;
using ByteBlock = std::vector<uint8_t>; // Array de longitud variable con datos binarios
using Blocks = std::vector<ByteBlock>; // Conjunto de ByteBlocks

const std::vector<std::string> DISK_NODES = {
    "http://127.0.0.1:5001", // 3 nodos para datos
    "http://127.0.0.1:5002",
    "http://127.0.0.1:5003",
    "http://127.0.0.1:5004" // 1 nodo para paridad
};

// mapa que guarda los tamaños de dato originales
std::unordered_map<std::string, size_t> file_original_sizes;

ByteBlock calculate_parity(const Blocks& blocks) { // Calcula el bloque de paridad XOR
    if (blocks.empty()) throw std::runtime_error("No blocks provided");
    ByteBlock parity(blocks[0].size(), 0); //Crea byteblock de paridad
    for (const auto& block : blocks) {
        for (size_t i = 0; i < block.size(); ++i) {
            parity[i] ^= block[i]; //Comparación XOR
        }
    }
    return parity;
}

void distribute_blocks(const Blocks& blocks, const std::string& file_id) {
    std::vector<Client> clients;
    for (const auto& url : DISK_NODES) {
        clients.push_back(Client(url.c_str()));
    }

    // envía 3 bloques a los primeros 3 nodos
    for (int i = 0; i < 3; i++) {
        json block_json;
        block_json["id"] = file_id + "_block" + std::to_string(i);
        block_json["data"] = blocks[i];

        auto res = clients[i].Post("/store", block_json.dump(), "application/json");
        if (!res) {
            std::cerr << "Connection failed to node " << (i+1) << "\n";
        } else if (res->status != 200) {
            std::cerr << "Error storing block " << i << " on node " << (i+1)
                      << ": " << res->status << " - " << res->body << "\n";
        }
    }

    // Calcula y envía paridad al nodo 4
    ByteBlock parity = calculate_parity({blocks[0], blocks[1], blocks[2]});
    json parity_json;
    parity_json["id"] = file_id + "_parity";
    parity_json["data"] = parity;

    auto res = clients[3].Post("/store", parity_json.dump(), "application/json");
    if (!res) {
        std::cerr << "Connection failed to node 4 (parity)\n";
    } else if (res->status != 200) {
        std::cerr << "Error storing parity on node 4: "
                  << res->status << " - " << res->body << "\n";
    }
}

Blocks reconstruct_file(const std::string& file_id) {
    std::vector<Client> clients;
    for (const auto& url : DISK_NODES) {
        clients.push_back(Client(url.c_str()));
    }

    Blocks recovered_blocks;

    // Intenta recuperar los 3 bloques
    for (int i = 0; i < 3; i++) {
        std::string block_id = file_id + "_block" + std::to_string(i);
        auto res = clients[i].Get(("/retrieve/" + block_id).c_str());

        if (res && res->status == 200) {
            try {
                auto json_data = json::parse(res->body);
                recovered_blocks.push_back(json_data["data"].get<ByteBlock>());
            } catch (const json::exception& e) {
                std::cerr << "JSON error for block " << i << ": " << e.what() << "\n";
            }
        } else {
            std::cerr << "Failed to get block " << i << ": "
                      << (res ? res->status : -1) << "\n";
        }
    }

    // Si falta un bloque, usa la paridad para reconstruirlo
    if (recovered_blocks.size() < 3) {
        auto parity_res = clients[3].Get(("/retrieve/" + file_id + "_parity").c_str());
        if (!parity_res || parity_res->status != 200) {
            throw std::runtime_error("Parity block retrieval failed");
        }

        auto parity_json = json::parse(parity_res->body);
        ByteBlock parity = parity_json["data"].get<ByteBlock>();
        ByteBlock recovered_block(parity.size(), 0);

        for (size_t j = 0; j < parity.size(); j++) {
            uint8_t value = parity[j];
            for (const auto& block : recovered_blocks) {
                value ^= block[j];
            }
            recovered_block[j] = value;
        }
        recovered_blocks.push_back(recovered_block);
    }

    return recovered_blocks;
}

int main() {
    Server svr;

    // upload endpoint
    svr.Post("/upload", [](const Request& req, Response& res) {
        // revisa si está vacío
        if (req.body.empty()) {
            res.status = 400;
            res.set_content("Missing file data", "text/plain");
            return;
        }

        try {
            // Convierte el contenido a bytes
            std::vector<uint8_t> file_data(req.body.begin(), req.body.end());
            size_t original_size = file_data.size();

            // Añade padding si es necesario (para división en 3 bloques)
            size_t pad = (3 - (original_size % 3)) % 3;
            file_data.insert(file_data.end(), pad, 0);

            // Divide en 3 bloques y distribuye
            size_t block_size = file_data.size() / 3;
            Blocks blocks;
            for (int i = 0; i < 3; i++) {
                auto start = file_data.begin() + i * block_size;
                blocks.push_back(ByteBlock(start, start + block_size));
            }

            std::string file_id = "file_" + std::to_string(time(nullptr));
            distribute_blocks(blocks, file_id);
            // Guarda el tamaño original (para eliminar padding después)
            file_original_sizes[file_id] = original_size;

            json response;
            response["file_id"] = file_id;
            response["status"] = "success";
            res.set_content(response.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    // Download endpoint
    svr.Get("/download/:file_id", [](const Request& req, Response& res) {
        std::string file_id = req.path_params.at("file_id");
        // Reconstruye los bloques (incluso si un nodo falló)
        Blocks blocks = reconstruct_file(file_id);

        // Combina y elimina el padding
        std::vector<uint8_t> full_data;
        for (const auto& block : blocks) {
            full_data.insert(full_data.end(), block.begin(), block.end());
        }

        // convert to original size
        if (file_original_sizes.count(file_id)) {
            size_t original_size = file_original_sizes[file_id];
            full_data.resize(original_size);

            // Determine content type (default to application/octet-stream)
            std::string content_type = "application/octet-stream";
            if (file_id.find(".pdf") != std::string::npos) {
                content_type = "application/pdf";
            }

            // Devuelve el archivo original
            res.set_content(
                std::string(full_data.begin(), full_data.end()),
                content_type
            );
        } else {
            res.status = 404;
            res.set_content("Original size not found", "text/plain");
        }
    });

    std::cout << "Controller running on port 8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}