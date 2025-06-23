#include <iostream>
#include <vector>
#include "httplib.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include "json.hpp"
#include "blocks.hpp"

namespace fs = std::filesystem;
using namespace httplib;
using json = nlohmann::json;
using ByteBlock = std::vector<uint8_t>;
using Blocks = std::vector<ByteBlock>;

const std::vector<std::string> DISK_NODES = {
    "http://localhost:5001",
    "http://localhost:5002",
    "http://localhost:5003",
    "http://localhost:5004"
};

ByteBlock calculate_parity(const Blocks& blocks) {
    if (blocks.empty()) throw std::runtime_error("No blocks provided");
    ByteBlock parity(blocks[0].size(), 0);
    for (const auto& block : blocks) {
        for (size_t i = 0; i < block.size(); ++i) {
            parity[i] ^= block[i];
        }
    }
    return parity;
}

void distribute_blocks(const Blocks& blocks, const std::string& file_id) {
    std::vector<Client> clients;
    for (const auto& url : DISK_NODES) {
        clients.push_back(Client(url.c_str()));
    }

    // Store data blocks
    for (int i = 0; i < 3; i++) {
        json block_json;
        block_json["id"] = file_id + "_block" + std::to_string(i);
        block_json["data"] = blocks[i];

        auto res = clients[i].Post("/store", block_json.dump(), "application/json");
        if (!res || res->status != 200) {
            std::cerr << "Error storing block " << i << " on node " << (i+1)
                      << ": " << (res ? res->status : -1) << "\n";
        }
    }

    // Calculate and store parity
    ByteBlock parity = calculate_parity({blocks[0], blocks[1], blocks[2]});
    json parity_json;
    parity_json["id"] = file_id + "_parity";
    parity_json["data"] = parity;

    auto res = clients[3].Post("/store", parity_json.dump(), "application/json");
    if (!res || res->status != 200) {
        std::cerr << "Error storing parity on node 4: " << (res ? res->status : -1) << "\n";
    }
}

Blocks reconstruct_file(const std::string& file_id) {
    std::vector<Client> clients;
    for (const auto& url : DISK_NODES) {
        clients.push_back(Client(url.c_str()));
    }

    Blocks recovered_blocks;

    // Retrieve data blocks
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

    // Recover missing blocks using parity
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

    // Upload endpoint
    svr.Post("/upload", [](const Request& req, Response& res) {
        std::string file_id = "file_" + std::to_string(time(nullptr));
        distribute_blocks(BLOCKS, file_id);

        json response;
        response["file_id"] = file_id;
        response["status"] = "success";
        res.set_content(response.dump(), "application/json");
    });

    // Download endpoint
    svr.Get("/download/:file_id", [](const Request& req, Response& res) {
        std::string file_id = req.path_params.at("file_id");
        Blocks blocks = reconstruct_file(file_id);

        std::string output_path = "reconstructed_" + file_id + ".pdf";
        std::ofstream out(output_path, std::ios::binary);
        for (const auto& block : blocks) {
            out.write(reinterpret_cast<const char*>(block.data()), block.size());
        }
        out.close();

        if (std::ifstream file(output_path, std::ios::binary); file) {
            res.set_content(std::string(
                (std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>()
            ), "application/pdf");
        } else {
            res.set_content(json{{"error", "File reconstruction failed"}}.dump(), "application/json");
        }
    });

    std::cout << "🚀 Controller running on port 8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}