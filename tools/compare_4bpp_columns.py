#!/usr/bin/env python3
"""
Compares the leftmost and rightmost columns of each frame in two 16x32 .4bpp sprite files (e.g., dusclops.4bpp and meowth.4bpp).
Reports if the left or right columns are different, which would indicate a horizontal shift.
"""
import sys
import os

def read_4bpp_frame_columns(filename, num_frames=9, frame_bytes=256):
    with open(filename, 'rb') as f:
        data = f.read()
    frames = []
    for frame in range(num_frames):
        start = frame * frame_bytes
        end = start + frame_bytes
        frame_data = data[start:end]
        # Each frame: 16x32 px, 2 tiles wide x 4 tiles tall, 8 tiles per frame
        # Each tile: 8x8 px, 32 bytes (4bpp)
        # Arrange tiles: left column (tiles 0,2,4,6), right column (tiles 1,3,5,7)
        left_col = []
        right_col = []
        for tile_row in range(4):
            tile_left = frame_data[(tile_row*2)*32:(tile_row*2+1)*32]
            tile_right = frame_data[(tile_row*2+1)*32:(tile_row*2+2)*32]
            # Get leftmost column (first pixel of each row in tile)
            for y in range(8):
                byte = tile_left[y*4:(y+1)*4]
                # Each byte contains 2 pixels (4bpp), so get high nibble
                left_col.append(byte[0] >> 4)
                # For right, get last pixel in row
                byte_r = tile_right[y*4:(y+1)*4]
                right_col.append(byte_r[3] & 0x0F)
        frames.append((left_col, right_col))
    return frames

def compare_columns(file1, file2):
    f1_cols = read_4bpp_frame_columns(file1)
    f2_cols = read_4bpp_frame_columns(file2)
    for i, ((l1, r1), (l2, r2)) in enumerate(zip(f1_cols, f2_cols)):
        left_diff = l1 != l2
        right_diff = r1 != r2
        print(f"Frame {i}: Left columns {'DIFFER' if left_diff else 'match'}, Right columns {'DIFFER' if right_diff else 'match'}")
        if left_diff or right_diff:
            print(f"  {os.path.basename(file1)} left:  {l1}")
            print(f"  {os.path.basename(file2)} left:  {l2}")
            print(f"  {os.path.basename(file1)} right: {r1}")
            print(f"  {os.path.basename(file2)} right: {r2}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python compare_4bpp_columns.py dusclops.4bpp meowth.4bpp")
        sys.exit(1)
    compare_columns(sys.argv[1], sys.argv[2])
