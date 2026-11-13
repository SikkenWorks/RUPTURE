#!/usr/bin/env python3
"""Pack the 20-frame V1.88.3 ENV artwork into the FM1's existing ROM arrays.

Only ENV is rewritten: LFO, SOUND SHAPE, audio, OTA and flash layout untouched.
Each frame is native 224x30 one-bit; no scaling or runtime raster operations.
"""

import hashlib
import json
import re
from pathlib import Path

from PIL import Image, ImageChops


ROOT = Path(__file__).resolve().parents[1]
NATIVE = ROOT / "vendor/etch-v1.88-source/firmware/fm1-native"
INPUT = ROOT / "assets/panel-animations-v1883/env"
HOLD = ROOT / "assets/panel-hold-v1883/env"
W, H, BYTES = 224, 30, 28


def pack(im):
    assert im.mode == "1" and im.size == (W, H)
    return bytes(
        sum((1 << (7 - bit)) if im.getpixel((x + bit, y)) else 0
            for bit in range(8))
        for y in range(H)
        for x in range(0, W, 8)
    )


def source_frames(path, n):
    return [Image.open(path / f"{i:02}.png").convert("1") for i in range(1, n + 1)]


def format_array(name, frames):
    lines = [f"static const uint8_t {name}[{len(frames)}][30][28]={{"]
    for frame in frames:
        raw = pack(frame)
        assert len(raw) == H * BYTES
        lines.append(" {")
        for y in range(H):
            lines.append("  {" + ",".join(f"0x{b:02x}" for b in raw[y*BYTES:(y+1)*BYTES]) + "},")
        lines.append(" },")
    lines.append("};")
    return "\n".join(lines)


def replace_array(header, name, frames):
    before = header.read_text(encoding="utf-8")
    needle = f"static const uint8_t {name}["
    start = before.index(needle)
    end = before.index("\n};", start) + len("\n};")
    original = before[start:end]
    bytes_before = bytes(int(v, 16) for v in re.findall(r"0x([0-9a-f]{2})", original))
    assert len(bytes_before) == len(frames) * H * BYTES
    content = format_array(name, frames)
    new = before[:start] + content + before[end:]
    header.write_text(new, encoding="utf-8")
    bytes_after = bytes(int(v, 16) for v in re.findall(r"0x([0-9a-f]{2})", content))
    assert bytes_after == b"".join(pack(frame) for frame in frames)
    return hashlib.sha256(bytes_before).hexdigest(), hashlib.sha256(bytes_after).hexdigest()


def main():
    frames = source_frames(INPUT, 20)
    hold = source_frames(HOLD, 8)
    for i in range(8):
        assert ImageChops.difference(frames[12+i], hold[i]).getbbox() is None
    assert frames[19].getpixel((112, 20)) == 0  # The open breach remains.
    old_lfo = (NATIVE / "rupture_panel_sprites.h").read_text().split("static const uint8_t ru_env_sprites[")[0]
    old_hold_lfo = (NATIVE / "rupture_hold_sprites.h").read_text().split("static const uint8_t ru_lfo_hold_sprites[")[1]
    changes = {}
    for name, header, src in (
        ("ru_env_sprites", NATIVE / "rupture_panel_sprites.h", frames),
        ("ru_env_hold_sprites", NATIVE / "rupture_hold_sprites.h", hold),
    ):
        before, after = replace_array(header, name, src)
        changes[name] = {"old_sha256": before, "new_sha256": after,
                         "frames": len(src), "bytes": len(src) * H * BYTES}
    assert (NATIVE / "rupture_panel_sprites.h").read_text().split("static const uint8_t ru_env_sprites[")[0] == old_lfo
    assert (NATIVE / "rupture_hold_sprites.h").read_text().split("static const uint8_t ru_lfo_hold_sprites[")[1] == old_hold_lfo
    (ROOT / "assets/panel-animations-v1883/ROM-MANIFEST.json").write_text(
        json.dumps({"version": "1.88.3", "size": [W, H],
                    "layout": "5x4", "frame_order": "01 -> 20 attack / 20 -> 01 release",
                    "hold_frame": "20 (smoke gone, hole stays)",
                    "source": "ENV-20-FRAMES.png", "rom_arrays": changes,
                    "ota_code_changed": False, "no_extra_rom_frames": True}, indent=2) + "\n",
        encoding="utf-8",
    )
    print("PASS V1.88.3 ENV packed: 20 full frames + 8 hold sprites; LFO byte-identical")


if __name__ == "__main__":
    main()
