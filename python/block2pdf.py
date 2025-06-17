import os
import sys
import json

def blocks_to_pdf(blocks, original_size, output_path):
    # Flatten all blocks into a single byte array
    all_bytes = bytearray()
    for block in blocks:
        all_bytes.extend(block)

    # Truncate to original size
    with open(output_path, 'wb') as f:
        f.write(all_bytes[:original_size])

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python block2pdf.py <input.json> <output.pdf>")
        sys.exit(1)

    input_json = sys.argv[1]
    output_pdf = sys.argv[2]

    if not os.path.exists(input_json):
        print(f"Error: {input_json} not found")
        sys.exit(1)

    try:
        with open(input_json, 'r') as f:
            data = json.load(f)

        blocks_to_pdf(data["blocks"], data["original_size"], output_pdf)
        print(f"Successfully reconstructed {output_pdf}")

    except Exception as e:
        print(f"Error processing JSON: {str(e)}")
        sys.exit(1)