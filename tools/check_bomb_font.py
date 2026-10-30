#!/usr/bin/env python3
"""Check the emitted C cmap and every 4bpp glyph, without Pillow or device access."""
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
font_dir = ROOT / 'assets/fonts'
source = (font_dir / 'bomb_font_18.c').read_text()
metadata = json.loads((font_dir / 'bomb-font-inventory.json').read_text())
strings = [json.loads(value) for value in re.findall(r'"[^"\n]*"', (ROOT / 'main/bomb_text.h').read_text())]
required = set(map(ord, ''.join(strings))) | set(range(32, 127))
# A Chinese literal added directly to the UI must not evade the inventory check.
for value in re.findall(r'"[^"\n]*"', (ROOT / 'main/bomb_ui.c').read_text()):
    required.update(ord(c) for c in json.loads(value) if ord(c) >= 128)
bitmap_text = re.search(r'glyph_bitmap\[\] = \{(.*?)\};', source, re.S)[1]
bitmap = bytes(int(value, 16) for value in re.findall(r'0x([0-9a-fA-F]{2})', bitmap_text))
assert hashlib.sha256(bitmap).hexdigest() == metadata['bitmap_sha256'], 'Bitmap hash mismatch'
assert hashlib.sha256((font_dir / 'bomb-ui-subset.ttf').read_bytes()).hexdigest() == metadata['source_sha256'], 'Source font hash mismatch'
entries = re.findall(r'\{\.bitmap_index=(\d+), \.adv_w=\d+, \.box_w=(\d+), \.box_h=(\d+), \.ofs_x=-?\d+, \.ofs_y=-?\d+\}, /\* U\+([0-9A-F]+) \*/', source)
actual = []
for entry, expected in zip(entries, metadata['glyphs']):
    start, width, height = map(int, entry[:3]); codepoint = int(entry[3], 16)
    assert (start, width, height, codepoint) == (expected['bitmap_index'], expected['width'], expected['height'], expected['codepoint']), 'Glyph descriptor mismatch'
    pixels = []
    for byte in bitmap[start:start + (width*height+1)//2]: pixels.extend((byte >> 4, byte & 15))
    assert hashlib.sha256(bytes(pixels[:width*height])).hexdigest() == expected['pixels_sha256'], f'Corrupt glyph U+{codepoint:04X}'
    actual.append(codepoint)
assert len(entries) == len(metadata['glyphs']) == len(required), 'Glyph count mismatch'
assert set(actual) == required, 'UI text and generated glyph inventory differ'
offsets = re.search(r'unicode_list\[\] = \{(.*?)\};', source, re.S)[1]
assert [int(n)+32 for n in re.findall(r'\d+', offsets)] == sorted(required), 'Cmap order mismatch'
assert 0x9f98 not in actual, 'Negative control unexpectedly present'
assert '.type=LV_FONT_FMT_TXT_CMAP_SPARSE_TINY' in source
print(f'Bomb font: {len(actual)} emitted glyphs, all C bitmap pixels, cmap, source SHA and negative control PASS')
