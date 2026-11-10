
import numpy as np
import sys

def read_ppm(filename):
    """Reads a binary PPM (P6) file and returns an RGB NumPy array normalized to [0, 1]."""
    with open(filename, 'rb') as f:
        header = f.readline().strip()
        if header != b'P6':
            raise ValueError(f"{filename} is not a binary PPM (P6) file")

        # Read width, height
        while True:
            line = f.readline()
            if line.startswith(b'#'):  # skip comments
                continue
            else:
                width, height = map(int, line.strip().split())
                break

        # Read max color value (typically 255)
        maxval = int(f.readline().strip())

        # Read pixel data
        pixel_data = np.frombuffer(f.read(), dtype=np.uint8)
        image = pixel_data.reshape((height, width, 3))
        return image.astype(np.float32) / maxval


def calculate_psnr(img1, img2):
    """Computes PSNR (Peak Signal-to-Noise Ratio) between two images."""
    if img1.shape != img2.shape:
        raise ValueError("Input images must have the same dimensions.")

    mse = np.mean((img1 - img2) ** 2)
    if mse == 0:
        return float('inf')  # identical images

    psnr = 10 * np.log10(1.0 / mse)
    return psnr


def main():
    if len(sys.argv) != 3:
        print("Usage: python compare_psnr.py <image1.ppm> <image2.ppm>")
        sys.exit(1)

    img1_path, img2_path = sys.argv[1], sys.argv[2]

    img1 = read_ppm(img1_path)
    img2 = read_ppm(img2_path)

    psnr = calculate_psnr(img1, img2)
    print(f"PSNR between '{img1_path}' and '{img2_path}': {psnr:.2f} dB")


if __name__ == "__main__":
    main()
