#include <iostream>
#include <vector>
#include <httplib.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "blocks.hpp"  // Generado por pdf2block

// Simple JSON implementation to avoid external dependencies
namespace simple_json {
    class value {
    public:
        std::string str_value;
        std::vector<uint8_t> bin_value;
        bool is_binary = false;

        value() = default;
        value(const std::string& s) : str_value(s) {}
        value(const std::vector<uint8_t>& b) : bin_value(b), is_binary(true) {}
    };

    class json {
    private:
        std::map<std::string, value> data;

    public:
        void operator[](const std::pair<std::string, std::vector<uint8_t>>& p) {
            data[p.first] = value(p.second);
        }

        void operator[](const std::pair<std::string, std::string>& p) {
            data[p.first] = value(p.second);
        }

        std::string dump() const {
            std::string result = "{";
            for (const auto& [key, val] : data) {
                result += "\"" + key + "\":";
                if (val.is_binary) {
                    result += "\"<binary data>\"";  // Simplified for this example
                } else {
                    result += "\"" + val.str_value + "\"";
                }
                result += ",";
            }
            if (!data.empty()) result.pop_back();  // Remove last comma
            result += "}";
            return result;
        }

        static json parse(const std::string& json_str) {
            // Simplified parser - in real code use a proper JSON library
            json result;
            // This is a placeholder - implement proper parsing if needed
            return result;
        }

        std::vector<uint8_t> get_binary(const std::string& key) const {
            if (data.find(key) != data.end() && data.at(key).is_binary) {
                return data.at(key).bin_value;
            }
            return {};
        }

        std::string get_string(const std::string& key) const {
            if (data.find(key) != data.end() && !data.at(key).is_binary) {
                return data.at(key).str_value;
            }
            return "";
        }
    };

    json object() { return json(); }
}

using json = simple_json::json;
namespace fs = std::filesystem;
using namespace httplib;
using ByteBlock = std::vector<uint8_t>;
using Blocks = std::vector<ByteBlock>;

// Configuración de nodos
const std::vector<std::string> DISK_NODES = {
    "http://localhost:5001",
    "http://localhost:5002",
    "http://localhost:5003",
    "http://localhost:5004"
};

ByteBlock calculate_parity(const Blocks& blocks) {
    ByteBlock parity(blocks[0].size(), 0);
    for (const auto& block : blocks) {
        for (size_t i = 0; i < block.size(); ++i) {
            parity[i] ^= block[i];
        }
    }
    return parity;
}

void distribute_blocks(const Blocks& blocks, const std::string& file_id) {
    Client clients[4] = {
        Client(DISK_NODES[0].c_str()),
        Client(DISK_NODES[1].c_str()),
        Client(DISK_NODES[2].c_str()),
        Client(DISK_NODES[3].c_str())
    };

    // Distribuir bloques de datos
    for (int i = 0; i < 3; i++) {
        json block_json;
        block_json[{"id", file_id + "_block" + std::to_string(i)}];
        block_json[{"data", blocks[i]}];

        clients[i].Post("/store", block_json.dump(), "application/json");
    }

    // Calcular y enviar paridad
    ByteBlock parity = calculate_parity({blocks[0], blocks[1], blocks[2]});
    json parity_json;
    parity_json[{"id", file_id + "_parity"}];
    parity_json[{"data", parity}];

    clients[3].Post("/store", parity_json.dump(), "application/json");
}

int main() {
    Server svr;

    // Endpoint para subir archivos
    svr.Post("/upload", [](const Request& req, Response& res) {
        // Convertir PDF a bloques (usando código existente)
        Blocks blocks = BLOCKS;

        // Generar ID único
        std::string file_id = "file_" + std::to_string(time(nullptr));

        // Distribuir a nodos
        distribute_blocks(blocks, file_id);

        json response;
        response[{"file_id", file_id}];
        res.set_content(response.dump(), "application/json");
    });

    // Endpoint para descargar archivos
    svr.Get("/download/:file_id", [](const Request& req, Response& res) {
        std::string file_id = req.path_params.at("file_id");
        Client clients[4] = {
            Client(DISK_NODES[0].c_str()),
            Client(DISK_NODES[1].c_str()),
            Client(DISK_NODES[2].c_str()),
            Client(DISK_NODES[3].c_str())
        };

        Blocks recovered_blocks;

        // Recuperar bloques de nodos
        for (int i = 0; i < 3; i++) {
            auto res_block = clients[i].Get(("/retrieve/" + file_id + "_block" + std::to_string(i)).c_str());
            if (res_block && res_block->status == 200) {
                // Simplified parsing - in real code you'd need proper JSON parsing
                recovered_blocks.push_back(ByteBlock(
                    res_block->body.begin(),
                    res_block->body.end()
                ));
            }
        }

        // Reconstruir PDF
        std::ofstream out("reconstructed.pdf", std::ios::binary);
        for (const auto& block : recovered_blocks) {
            out.write(reinterpret_cast<const char*>(block.data()), block.size());
        }
        out.close();

        // Enviar archivo reconstruido
        if (std::ifstream file("reconstructed.pdf", std::ios::binary); file) {
            res.set_content(std::string(
                (std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>()
            ), "application/pdf");
        } else {
            res.set_content("File reconstruction failed", "text/plain");
        }
    });

    std::cout << "Controller node running on port 8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}