from __future__ import annotations

import sys
from pathlib import Path
from typing import Tuple

import numpy as np
from PIL import Image

_LUMA_WEIGHTS: Tuple[float, float, float] = (0.299, 0.587, 0.114)

def _load_image(path: Path) -> Image.Image:
    if path.exists():
        return Image.open(path)
    raise FileNotFoundError(f"Input image not found: {path}")

def convert_to_grayscale(src_path: Path, dst_path: Path) -> None:
    img = _load_image(src_path).convert("RGB")
    rgb = np.asarray(img, dtype=np.uint8)

    luma = np.rint(rgb @ np.array(_LUMA_WEIGHTS, dtype=np.float32)).astype(np.uint8)

    gray_img = Image.fromarray(luma, mode="L")
    dst_path.parent.mkdir(parents=True, exist_ok=True)
    gray_img.save(dst_path)

def _main(argv: list[str]) -> int:
    if len(argv) != 3:
        sys.stderr.write("Usage: python noirify.py <input_path> <output_path>\n")
        return 1

    src = Path(argv[1])
    dst = Path(argv[2])
    try:
        convert_to_grayscale(src, dst)
    except Exception as exc:
        sys.stderr.write(f"Error: {exc}\n")
        return 1
    return 0

if __name__ == "__main__":
    raise SystemExit(_main(sys.argv))