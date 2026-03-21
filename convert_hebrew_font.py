#!/usr/bin/env python3
"""
Convert Hebrew TTF font to Adafruit GFXfont format.
"""
import freetype
import os

FONT_PATH = '/tmp/hebrew.ttf'
OUTPUT_PATH = '/home/alon/Projects/kindle/components/hebrew_font/hebrew_font.h'
FONT_SIZE = 16

HEBREW_START = 0x0590
HEBREW_END = 0x05FF

def render_glyph(face, codepoint):
    char = chr(codepoint)
    glyph_index = face.get_char_index(codepoint)
    if glyph_index == 0:
        return None

    face.load_char(char, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
    slot = face.glyph
    bitmap = slot.bitmap
    pitch = abs(bitmap.pitch)

    pixels = []
    for row in range(bitmap.rows):
        row_data = []
        for col in range(bitmap.width):
            byte_idx = col // 8
            bit_idx = 7 - (col % 8)
            byte_val = bitmap.buffer[row * pitch + byte_idx]
            row_data.append(1 if (byte_val & (1 << bit_idx)) else 0)
        pixels.append(row_data)

    return {
        'width': bitmap.width,
        'rows': bitmap.rows,
        'buffer': bitmap.buffer,
        'pitch': pitch,
        'top': slot.bitmap_top,
        'left': slot.bitmap_left,
        'advance': slot.advance.x,
        'pixels': pixels,
    }

def pack_bits(pixels, width):
    packed = []
    for row in pixels:
        row_bytes = []
        for i in range(0, width, 8):
            byte = 0
            for j in range(8):
                if i + j < len(row) and row[i + j]:
                    byte |= (1 << (7 - j))
            row_bytes.append(byte)
        packed.extend(row_bytes)
    return bytes(packed)

def main():
    face = freetype.Face(FONT_PATH)
    face.set_pixel_sizes(0, FONT_SIZE)

    glyphs = []
    bitmaps = []
    total_bitmap_size = 0

    print("Rendering %d Hebrew glyphs at %dpx..." % (HEBREW_END - HEBREW_START + 1, FONT_SIZE))

    for cp in range(HEBREW_START, HEBREW_END + 1):
        rendered = render_glyph(face, cp)
        if rendered and rendered['width'] > 0 and rendered['rows'] > 0:
            packed = pack_bits(rendered['pixels'], rendered['width'])
            glyphs.append({
                'codepoint': cp,
                'width': rendered['width'],
                'rows': rendered['rows'],
                'top': rendered['top'],
                'left': rendered['left'],
                'advance': rendered['advance'],
                'bitmapOffset': total_bitmap_size,
                'bitmap': packed,
            })
            bitmaps.append(packed)
            total_bitmap_size += len(packed)
        else:
            glyphs.append(None)

    rendered_count = sum(1 for g in glyphs if g)
    print("Rendered %d glyphs, bitmap size: %d bytes" % (rendered_count, total_bitmap_size))

    advance_y = FONT_SIZE + 2

    with open(OUTPUT_PATH, 'w') as f:
        f.write('#ifndef HEBREW_FONT_H\n')
        f.write('#define HEBREW_FONT_H\n\n')
        f.write('#include <stdint.h>\n\n')

        f.write('typedef struct {\n')
        f.write('  uint16_t bitmapOffset;\n')
        f.write('  uint8_t width;\n')
        f.write('  uint8_t height;\n')
        f.write('  uint8_t xAdvance;\n')
        f.write('  int8_t xOffset;\n')
        f.write('  int8_t yOffset;\n')
        f.write('} GFXglyph;\n\n')

        f.write('typedef struct {\n')
        f.write('  uint8_t* bitmap;\n')
        f.write('  GFXglyph* glyph;\n')
        f.write('  uint16_t first;\n')
        f.write('  uint16_t last;\n')
        f.write('  uint8_t yAdvance;\n')
        f.write('} GFXfont;\n\n')

        f.write('static const uint8_t hebrew_font_bitmaps[] = {\n')
        for i, glyph in enumerate(glyphs):
            if glyph is None:
                f.write('  /* 0x%04X placeholder */\n' % (HEBREW_START + i))
                for j in range(0, 32, 8):
                    f.write('  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\n')
            else:
                f.write('  /* 0x%04X %dx%d top=%d left=%d adv=%d */\n' % (
                    glyph['codepoint'], glyph['width'], glyph['rows'],
                    glyph['top'], glyph['left'], glyph['advance']))
                for j in range(0, len(glyph['bitmap']), 8):
                    chunk = glyph['bitmap'][j:j+8]
                    f.write('  ' + ', '.join('0x%02X' % b for b in chunk) + ',\n')
        f.write('};\n\n')

        f.write('static const GFXglyph hebrew_font_glyphs[] = {\n')
        for glyph in glyphs:
            if glyph is None:
                f.write('  { 0, 0, 0, 0, 0, 0 },\n')
            else:
                y_off = FONT_SIZE - glyph['top']
                x_adv = (glyph['advance'] + 31) // 32
                f.write('  { %d, %d, %d, %d, %d, %d },\n' % (
                    glyph['bitmapOffset'],
                    glyph['width'],
                    glyph['rows'],
                    x_adv,
                    glyph['left'],
                    y_off
                ))
        f.write('};\n\n')

        f.write('static const GFXfont hebrewFont = {\n')
        f.write('  (uint8_t*)hebrew_font_bitmaps,\n')
        f.write('  (GFXglyph*)hebrew_font_glyphs,\n')
        f.write('  0x%04X,\n' % HEBREW_START)
        f.write('  0x%04X,\n' % HEBREW_END)
        f.write('  %d // yAdvance\n' % advance_y)
        f.write('};\n\n')

        f.write('#endif\n')

    size = os.path.getsize(OUTPUT_PATH)
    print("Generated: %s (%d bytes)" % (OUTPUT_PATH, size))
    print("Font: first=0x%04X, last=0x%04X, yAdvance=%d" % (HEBREW_START, HEBREW_END, advance_y))

if __name__ == '__main__':
    main()
