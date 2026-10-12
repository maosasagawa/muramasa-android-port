#!/usr/bin/env python3
# Packer for the NPA\x01 v1 archive format read by libnpa's INipaFile
# (Muramasa variant). Writes Compress=0, Encrypt=0 archives: entry names
# are stored GBK-encoded and XOR-shifted by the inverse of
# INipaFile::FilenameCrypt; file data is stored raw.
import os
import struct
import sys

KEY1 = 0
KEY2 = 0


def i32(v):
    v &= 0xFFFFFFFF
    return v - 0x100000000 if v & 0x80000000 else v


def filename_crypt(ci, fi):
    # Mirrors INipaFile::FilenameCrypt for Encrypt=0 (Temp = Key1*Key2)
    key = 0xFC * ci
    temp = i32(KEY1 * KEY2)
    key -= temp >> 24
    key -= temp >> 16
    key -= temp >> 8
    key -= temp & 0xFF
    key -= fi >> 24
    key -= fi >> 16
    key -= fi >> 8
    key -= fi
    return key & 0xFF


def crypt_name(plain_bytes, fi):
    out = bytearray(len(plain_bytes))
    for i, b in enumerate(plain_bytes):
        out[i] = (b - filename_crypt(i, fi)) & 0xFF
    return bytes(out)


def main():
    src = sys.argv[1].rstrip("/")
    dst = sys.argv[2]
    files = []
    for root, _dirs, names in os.walk(src):
        for n in names:
            full = os.path.join(root, n)
            rel = os.path.relpath(full, src)
            files.append((rel.replace("/", "\\"), full))
    files.sort(key=lambda x: x[0])

    entries = []
    for i, (rel, full) in enumerate(files):
        # Archive names are stored in the game charset
        raw = rel.encode("gbk", errors="strict")
        entries.append((crypt_name(raw, i), i, os.path.getsize(full), full))

    table = bytearray()
    for cname, i, size, _full in entries:
        table += struct.pack("<I", len(cname)) + cname
        table += struct.pack("<BIIII", 0, i, 0, size, size)

    start = len(table)  # data section starts at 0x29 + Start
    header = struct.pack(
        "<7siiBBIIIQI",
        b"NPA\x01\x00\x00\x00",
        KEY1,
        KEY2,
        0,  # compress
        0,  # encrypt
        len(entries),  # total count
        0,  # folder count
        len(entries),  # file count
        0,  # unk u64
        start,
    )
    assert len(header) == 0x29

    # fix up offsets now that table size is known
    pos = 0
    off_entries = []
    for cname, i, size, _full in entries:
        off_entries.append((cname, i, pos, size))
        pos += size
    table = bytearray()
    for (cname, i, offset, size), (_c, _i, _s, _f) in zip(off_entries, entries):
        table += struct.pack("<I", len(cname)) + cname
        table += struct.pack("<BIIII", 0, i, offset, size, size)
    header = header[:37] + struct.pack("<I", len(table))  # refresh Start

    with open(dst, "wb") as out:
        out.write(header)
        out.write(table)
        for (cname, i, offset, size), (_c, _i2, _s2, full) in zip(off_entries, entries):
            with open(full, "rb") as f:
                while True:
                    chunk = f.read(1 << 20)
                    if not chunk:
                        break
                    out.write(chunk)
    print(f"{dst}: {len(entries)} files")


if __name__ == "__main__":
    main()
