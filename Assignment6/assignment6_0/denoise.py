"""
denoise.py — Bilateral filter denoiser for path-traced PPM images.

Usage:
    python denoise.py <input.ppm> [output.ppm] [--sigma_s S] [--sigma_r R] [--radius R]

Arguments:
    input.ppm       Path to the noisy rendered PPM file (P6 binary format).
    output.ppm      Output path (default: <input>_denoised.ppm).
    --sigma_s       Spatial Gaussian sigma in pixels (default: 3.0).
    --sigma_r       Range (color) Gaussian sigma, 0–1 scale (default: 0.15).
    --radius        Filter half-window size in pixels (default: 7).

Example:
    python denoise.py ./build/binary_diffuse.ppm ./build/binary_diffuse_denoised.ppm
    python denoise.py ./build/binary.ppm --sigma_s 5 --sigma_r 0.1
"""

import argparse
import math
import os
import struct
import sys


# ── PPM I/O ──────────────────────────────────────────────────────────────────

def read_ppm(path):
    """Read a P6 (binary) PPM file. Returns (pixels, width, height) where
    pixels is a flat list of floats in [0, 1], stored as R G B R G B ..."""
    with open(path, "rb") as f:
        # Parse ASCII header
        def next_token():
            token = b""
            while True:
                ch = f.read(1)
                if ch in (b" ", b"\t", b"\n", b"\r"):
                    if token:
                        return token.decode()
                elif ch == b"#":
                    f.readline()  # skip comment
                else:
                    token += ch

        magic  = next_token()
        if magic != "P6":
            raise ValueError(f"Expected P6 PPM, got '{magic}'")
        width  = int(next_token())
        height = int(next_token())
        maxval = int(next_token())
        # next_token() already consumed the whitespace after maxval

        raw = f.read(width * height * 3)

    scale = 1.0 / maxval
    pixels = [b * scale for b in raw]
    return pixels, width, height


def write_ppm(path, pixels, width, height):
    """Write a P6 PPM file from a flat float list (values in [0, 1])."""
    with open(path, "wb") as f:
        f.write(f"P6\n{width} {height}\n255\n".encode())
        for v in pixels:
            v = max(0.0, min(1.0, v))
            f.write(struct.pack("B", int(v * 255)))


# ── Bilateral filter ─────────────────────────────────────────────────────────

def bilateral_filter(pixels, width, height, sigma_s, sigma_r, radius):
    """
    Apply a bilateral filter to the image.

    For each pixel p, the output is a weighted average of nearby pixels q:
        weight(p, q) = G_s(||p - q||) * G_r(||color(p) - color(q)||)
    where G_s is a spatial Gaussian and G_r is a range (color) Gaussian.

    This smooths noise while preserving edges — nearby pixels with similar
    color contribute strongly, while pixels across an edge are down-weighted
    by the range kernel.
    """
    inv_2ss = 1.0 / (2.0 * sigma_s * sigma_s)
    inv_2sr = 1.0 / (2.0 * sigma_r * sigma_r)

    # Precompute spatial Gaussian weights for the kernel window
    spatial = {}
    for dy in range(-radius, radius + 1):
        for dx in range(-radius, radius + 1):
            spatial[(dy, dx)] = math.exp(-(dx*dx + dy*dy) * inv_2ss)

    out = [0.0] * (width * height * 3)

    for y in range(height):
        for x in range(width):
            idx = (y * width + x) * 3
            cr, cg, cb = pixels[idx], pixels[idx+1], pixels[idx+2]

            wr = wg = wb = 0.0
            sr = sg = sb = 0.0

            for dy in range(-radius, radius + 1):
                ny = y + dy
                if ny < 0 or ny >= height:
                    continue
                for dx in range(-radius, radius + 1):
                    nx = x + dx
                    if nx < 0 or nx >= width:
                        continue

                    nidx = (ny * width + nx) * 3
                    nr, ng, nb = pixels[nidx], pixels[nidx+1], pixels[nidx+2]

                    # Range weight: per-channel so color edges are respected
                    ws = spatial[(dy, dx)]
                    wwr = ws * math.exp(-((nr - cr) ** 2) * inv_2sr)
                    wwg = ws * math.exp(-((ng - cg) ** 2) * inv_2sr)
                    wwb = ws * math.exp(-((nb - cb) ** 2) * inv_2sr)

                    sr += wwr * nr;  wr += wwr
                    sg += wwg * ng;  wg += wwg
                    sb += wwb * nb;  wb += wwb

            out[idx]   = sr / wr if wr > 0 else cr
            out[idx+1] = sg / wg if wg > 0 else cg
            out[idx+2] = sb / wb if wb > 0 else cb

        # Progress
        if (y + 1) % max(1, height // 20) == 0:
            pct = (y + 1) / height * 100
            print(f"\r  Denoising... {pct:5.1f}%", end="", flush=True)

    print()
    return out


# ── PSNR helper ──────────────────────────────────────────────────────────────

def psnr(a, b):
    """Compute PSNR between two flat float pixel lists (values in [0, 1])."""
    mse = sum((x - y) ** 2 for x, y in zip(a, b)) / len(a)
    if mse == 0:
        return float("inf")
    return 10 * math.log10(1.0 / mse)


# ── Main ─────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="Bilateral filter denoiser for path-traced PPM images."
    )
    parser.add_argument("input",             help="Noisy input PPM file")
    parser.add_argument("output", nargs="?", help="Output PPM file (optional)")
    parser.add_argument("--sigma_s", type=float, default=3.0,
                        help="Spatial Gaussian sigma in pixels (default: 3.0)")
    parser.add_argument("--sigma_r", type=float, default=0.15,
                        help="Range Gaussian sigma in [0,1] (default: 0.15)")
    parser.add_argument("--radius",  type=int,   default=7,
                        help="Filter half-window radius in pixels (default: 7)")
    parser.add_argument("--reference", default=None,
                        help="Optional reference PPM to compute PSNR before/after")
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Error: '{args.input}' not found.")
        sys.exit(1)

    # Default output path
    if args.output is None:
        base, ext = os.path.splitext(args.input)
        args.output = base + "_denoised" + (ext if ext else ".ppm")

    print(f"Input : {args.input}")
    print(f"Output: {args.output}")
    print(f"Filter: bilateral  sigma_s={args.sigma_s}  sigma_r={args.sigma_r}  radius={args.radius}")

    noisy, width, height = read_ppm(args.input)
    print(f"Image : {width}x{height}")

    denoised = bilateral_filter(noisy, width, height,
                                sigma_s=args.sigma_s,
                                sigma_r=args.sigma_r,
                                radius=args.radius)

    write_ppm(args.output, denoised, width, height)
    print(f"Saved denoised image to '{args.output}'")

    # Optional PSNR comparison against a reference image
    if args.reference:
        if not os.path.isfile(args.reference):
            print(f"Warning: reference '{args.reference}' not found, skipping PSNR.")
        else:
            ref, rw, rh = read_ppm(args.reference)
            if rw != width or rh != height:
                print("Warning: reference size differs, skipping PSNR.")
            else:
                print(f"\nPSNR (noisy   vs reference): {psnr(noisy,    ref):.2f} dB")
                print(f"PSNR (denoised vs reference): {psnr(denoised, ref):.2f} dB")


if __name__ == "__main__":
    main()
