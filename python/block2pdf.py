import os
import sys
import json

def blocks_to_pdf(blocks, original_size, output_path):
    all_bytes = bytearray()
    for block in blocks:
        # Convert all elements to integers then to bytes
        byte_block = bytes(int(x) for x in block)
        all_bytes.extend(byte_block)

    # Truncate to original size
    with open(output_path, 'wb') as f:
        f.write(all_bytes[:original_size])
    return len(all_bytes)

if __name__ == "__main__":
    print("\n=== block2pdf.py starting ===")
    print(f"Arguments: {sys.argv}")
    print(f"CWD: {os.getcwd()}")

    if len(sys.argv) != 3:
        print(f"Error: Expected 2 arguments, got {len(sys.argv)-1}")
        print("Usage: python block2pdf.py <input.json> <output.pdf>")
        sys.exit(1)

    input_json = sys.argv[1]
    output_pdf = sys.argv[2]

    print(f"Input JSON: {input_json}")
    print(f"Output PDF: {output_pdf}")

    if not os.path.exists(input_json):
        print(f"Error: {input_json} not found")
        sys.exit(1)

    try:
        print(f"Loading JSON...")
        with open(input_json, 'r') as f:
            data = json.load(f)

        original_size = data["original_size"]
        blocks = data["blocks"]
        print(f"Original size: {original_size} bytes")
        print(f"Blocks count: {len(blocks)}")
        if blocks:
            print(f"First block size: {len(blocks[0])} bytes")
            print(f"First block start: {blocks[0][:5]}...")

        total_bytes = blocks_to_pdf(blocks, original_size, output_pdf)
        print(f"Total bytes processed: {total_bytes}")

        if os.path.exists(output_pdf):
            size = os.path.getsize(output_pdf)
            print(f"PDF created at {os.path.abspath(output_pdf)}")
            print(f"PDF size: {size} bytes")
            if size == original_size:
                print("Size matches original!")
            else:
                print(f"Warning: Size mismatch! Expected {original_size} bytes")
        else:
            print("Error: PDF was not created")

        print("=== Reconstruction complete ===\n")
    except Exception as e:
        print(f"Error: {str(e)}")
        import traceback
        traceback.print_exc()
        sys.exit(1)