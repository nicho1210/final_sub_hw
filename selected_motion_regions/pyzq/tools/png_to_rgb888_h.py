from PIL import Image
import sys
from pathlib import Path

W, H = 1280, 720

def main():
    if len(sys.argv) != 3:
        print("Usage: python png_to_rgb888_h.py <input_image> <output_header>")
        sys.exit(1)

    input_image = sys.argv[1]
    output_header = sys.argv[2]

    img = Image.open(input_image).convert("RGB").resize((W, H))
    data = img.tobytes()

    out_path = Path(output_header)
    out_path.parent.mkdir(parents=True, exist_ok=True)

    with open(out_path, "w", encoding="utf-8") as f:
        f.write("#pragma once\n")
        f.write(f"#define FRAME_W {W}\n")
        f.write(f"#define FRAME_H {H}\n")
        f.write(f"#define FRAME_STRIDE ({W} * 3)\n")
        f.write(f"#define FRAME_BYTES {len(data)}\n")
        f.write("static const unsigned char frame_rgb888[] = {\n")

        for i, b in enumerate(data):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"0x{b:02x}, ")
            if i % 12 == 11:
                f.write("\n")

        f.write("\n};\n")

    print(f"Generated: {out_path}")
    print(f"Image size: {W}x{H}")
    print(f"Bytes: {len(data)}")

if __name__ == "__main__":
    main()