"""Pack pre-rendered PNGs into a Windows .ico and a macOS .icns (PNG payloads, stdlib only).
usage: pack-icons.py <png-dir> <out.ico> <out.icns>   (png-dir holds c<size>.png)"""
import struct, sys

png_dir, ico_out, icns_out = sys.argv[1:4]
def png(s): return open(f"{png_dir}/c{s}.png", "rb").read()

# ICO: every entry a PNG (Vista+); 256 is encoded as 0 in the directory.
sizes = [16, 20, 24, 32, 40, 48, 64, 256]
blobs = [png(s) for s in sizes]
off = 6 + 16 * len(sizes)
head = struct.pack("<HHH", 0, 1, len(sizes)); body = b""
for s, b in zip(sizes, blobs):
    d = 0 if s >= 256 else s
    head += struct.pack("<BBBBHHII", d, d, 0, 0, 1, 32, len(b), off + len(body))
    body += b
open(ico_out, "wb").write(head + body)

# ICNS: type code -> pixel size
codes = [("icp4", 16), ("icp5", 32), ("icp6", 64), ("ic07", 128), ("ic08", 256), ("ic09", 512),
         ("ic10", 1024), ("ic11", 32), ("ic12", 64), ("ic13", 256), ("ic14", 512)]
chunks = b"".join(c.encode() + struct.pack(">I", 8 + len(png(s))) + png(s) for c, s in codes)
open(icns_out, "wb").write(b"icns" + struct.pack(">I", 8 + len(chunks)) + chunks)
