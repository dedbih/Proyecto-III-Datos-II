#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include "blocks.hpp"  // Include generated header

using ByteBlock = std::vector<uint8_t>;
using Blocks = std::vector<ByteBlock>;

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

void save_to_json(const Blocks& blocks, size_t original_size, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }

    file << "{\n";
    file << "  \"original_size\": " << original_size << ",\n";
    file << "  \"blocks\": [\n";

    for (size_t i = 0; i < blocks.size(); ++i) {
        file << "    [";
        for (size_t j = 0; j < blocks[i].size(); ++j) {
            file << static_cast<int>(blocks[i][j]);
            if (j < blocks[i].size() - 1) file << ", ";
        }
        file << "]";
        if (i < blocks.size() - 1) file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";
}

int main() {
    try {
        std::cout << "Starting RAID operations...\n";

        // Debug output
        std::cout << "Total blocks: " << BLOCKS.size() << "\n";
        std::cout << "Original size: " << ORIGINAL_SIZE << " bytes\n";

        if (BLOCKS.size() < 3) {
            throw std::runtime_error("Need at least 3 blocks for RAID 5");
        }

        // Calculate parity
        ByteBlock parity = calculate_parity({BLOCKS[0], BLOCKS[1], BLOCKS[2]});
        std::cout << "Parity calculation successful\n";

        // Save data for reconstruction
        std::string json_path = "blocks.json";
        save_to_json(BLOCKS, ORIGINAL_SIZE, json_path);
        std::cout << "Saved block data to " << json_path << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}