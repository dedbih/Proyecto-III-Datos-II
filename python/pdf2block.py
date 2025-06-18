import os
import sys

def pdf_to_blocks(pdf_path, block_size=4096):
    with open(pdf_path, 'rb') as f:
        data = f.read()
    blocks = []
    for i in range(0, len(data), block_size):
        block = data[i:i+block_size]
        if len(block) < block_size:
            block += b'\x00' * (block_size - len(block))
        blocks.append(list(block))
    return blocks, len(data)

def save_to_header(blocks, original_size, output_path):
    abs_path = os.path.abspath(output_path)
    output_dir = os.path.dirname(abs_path)
    os.makedirs(output_dir, exist_ok=True)

    with open(abs_path, 'w') as f:
        f.write("#pragma once\n#include <vector>\n\n")
        f.write(f"const size_t ORIGINAL_SIZE = {original_size};\n")
        f.write("using ByteBlock = std::vector<uint8_t>;\n")
        f.write("using Blocks = std::vector<ByteBlock>;\n\n")
        f.write("const Blocks BLOCKS = {\n")
        for block in blocks:
            f.write("    {" + ", ".join(f"0x{b:02x}" for b in block) + "},\n")
        f.write("};\n")
    print(f"Saved header to: {abs_path}")
    print(f"Header size: {os.path.getsize(abs_path) / 1024:.2f} KB")
    print(f"Blocks: {len(blocks)}, Original size: {original_size} bytes")
    if blocks:
        print(f"Block size: {len(blocks[0])} bytes")
        print(f"First 5 bytes: {blocks[0][:5]}")

if __name__ == "__main__":
    print("\n=== pdf2block.py starting ===")
    print(f"Arguments: {sys.argv}")
    print(f"CWD: {os.getcwd()}")

    if len(sys.argv) != 3:
        print("Usage: python pdf2block.py <input.pdf> <output.hpp>")
        sys.exit(1)

    input_pdf = os.path.abspath(sys.argv[1])
    output_header = os.path.abspath(sys.argv[2])

    print(f"Input PDF: {input_pdf}")
    print(f"Output header: {output_header}")

    if not os.path.exists(input_pdf):
        print(f"Error: {input_pdf} not found")
        sys.exit(1)

    try:
        print(f"Processing PDF...")
        blocks, original_size = pdf_to_blocks(input_pdf)
        print(f"Converted PDF into {len(blocks)} blocks")
        save_to_header(blocks, original_size, output_header)
        print(f"Successfully generated header\n")
    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)