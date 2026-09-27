# /// script
# dependencies = ["Pillow"]
# ///

from PIL import Image
from pathlib import Path


input_dir_path = Path(__file__).parent / "../images"
output_dir_path = Path(__file__).parent / "../src/sprites"

def bswap16(value):
  return ((value & 0xFF) << 8) | ((value >> 8) & 0xFF)

def rgb565(r, g, b):
  value = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
  return bswap16(value)

def png_to_c_array(filename):
  name = Path(filename).stem

  img = Image.open(filename)
  img = img.convert("RGB")
  width, height = img.size
  pixels = list(img.get_flattened_data())
  array = []
  for r, g, b in pixels:
    array.append(rgb565(r, g, b))
  return name, array, width, height

def build_header(image):
  name, array, width, height = image

  const_name = f"SPRITE_{name.upper()}"
  sprite_data_name = f"sprite_{name.lower()}"

  header = f"#include <cstdint>\n\n#define {const_name}_WIDTH {width}\n#define {const_name}_HEIGHT {height}\n#define {const_name}_NAME \"{name}\"\n"
  header += f"const uint16_t {sprite_data_name}_data[] = {{\n"
  for i, value in enumerate(array):
    if i % 8 == 0:
      header += "  "
    header += f"0x{value:04X}, "
    if (i + 1) % 8 == 0:
      header += "\n"
  header += "\n};\n"
  return header

images = []

for image in input_dir_path.glob("*.png"):
  name, array, width, height = png_to_c_array(image)
  images.append((name, array, width, height))

print("Converted images:", len(images))
print("Images details:")
for i, (name, array, width, height) in enumerate(images):
  print(f"Image {i} ({name}): {width}x{height}, {len(array)} pixels")
  header = build_header((name, array, width, height))
  output_file_path = output_dir_path / f"sprite_{name}.h"
  with open(output_file_path, "w") as f:
    f.write(header)
    print(f"Wrote header to {output_file_path}")