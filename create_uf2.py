#!/usr/bin/env python3
import struct
import sys

def create_uf2(bin_file, uf2_file):
    UF2_MAGIC_START0 = 0x0A324655
    UF2_MAGIC_START1 = 0x9E5D5157
    UF2_MAGIC_END = 0x0AB16F30
    UF2_FLAG_FAMILYID_PRESENT = 0x2000
    RP2040_FAMILY_ID = 0xe48bff56
    
    with open(bin_file, 'rb') as f:
        data = f.read()
    
    blocks = []
    addr = 0x10000000  # Flash start address for RP2040
    
    for i in range(0, len(data), 256):
        chunk = data[i:i+256]
        if len(chunk) < 256:
            chunk += b'\x00' * (256 - len(chunk))
        
        block = struct.pack('<LLLLLLLL',
            UF2_MAGIC_START0,
            UF2_MAGIC_START1,
            UF2_FLAG_FAMILYID_PRESENT,
            addr + i,
            256,
            i // 256,
            (len(data) + 255) // 256,
            RP2040_FAMILY_ID
        )
        block += chunk
        block += b'\x00' * (512 - 32 - 256)
        block += struct.pack('<L', UF2_MAGIC_END)
        blocks.append(block)
    
    with open(uf2_file, 'wb') as f:
        for block in blocks:
            f.write(block)

if __name__ == '__main__':
    create_uf2('pico_uart_ble_bridge.bin', 'pico_uart_ble_bridge.uf2')
    print("UF2 file created successfully!")
