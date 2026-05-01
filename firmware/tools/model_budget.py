#!/usr/bin/env python3
"""
model_budget.py — ESP32-S3 memory budget calculator for embedded vision.

Usage:
    python3 model_budget.py --psram_kb 8192 --camera_res 320x240 \
                            --camera_format grayscale --model_kb 300
"""

import argparse
import sys


PIXEL_BYTES = {"grayscale": 1, "rgb888": 3, "jpeg": 0}  # jpeg is variable
JPEG_ESTIMATE_RATIO = 0.05  # ~5% of raw RGB at quality=12


def parse_res(res: str) -> tuple[int, int]:
    parts = res.lower().split("x")
    if len(parts) != 2:
        sys.exit(f"Bad resolution '{res}' — expected WxH, e.g. 320x240")
    return int(parts[0]), int(parts[1])


def frame_buffer_bytes(width: int, height: int, fmt: str, fb_count: int) -> int:
    pixels = width * height
    if fmt == "grayscale":
        raw = pixels * 1
    elif fmt == "rgb888":
        raw = pixels * 3
    elif fmt == "jpeg":
        raw = int(pixels * 3 * JPEG_ESTIMATE_RATIO)
        raw = max(raw, 8192)   # minimum DMA buffer
    else:
        sys.exit(f"Unknown format '{fmt}' — choose: grayscale, rgb888, jpeg")
    return raw * fb_count


def print_bar(label: str, used_kb: float, total_kb: float, width: int = 40) -> None:
    pct = used_kb / total_kb
    filled = int(pct * width)
    bar = "█" * filled + "░" * (width - filled)
    print(f"  {label:<22} [{bar}] {used_kb:6.0f} / {total_kb:.0f} KB  ({pct*100:.1f}%)")


def main() -> None:
    ap = argparse.ArgumentParser(description="ESP32-S3 PSRAM budget calculator")
    ap.add_argument("--psram_kb",       type=int,   default=8192,
                    help="Total PSRAM in KB (default: 8192 for 8MB)")
    ap.add_argument("--camera_res",     type=str,   default="320x240",
                    help="Camera resolution WxH (default: 320x240)")
    ap.add_argument("--camera_format",  type=str,   default="grayscale",
                    choices=["grayscale", "rgb888", "jpeg"],
                    help="Pixel format (default: grayscale)")
    ap.add_argument("--fb_count",       type=int,   default=2,
                    help="Number of frame buffers (default: 2)")
    ap.add_argument("--model_kb",       type=int,   default=300,
                    help="Model size in KB (default: 300 for MobileNetV1 0.25x INT8)")
    ap.add_argument("--arena_kb",       type=int,   default=0,
                    help="Tensor arena KB (0 = auto-estimate as 2× model)")
    args = ap.parse_args()

    width, height = parse_res(args.camera_res)
    fb_bytes = frame_buffer_bytes(width, height, args.camera_format, args.fb_count)
    fb_kb = fb_bytes / 1024

    model_kb = args.model_kb
    arena_kb = args.arena_kb if args.arena_kb > 0 else model_kb * 2
    overhead_kb = 128   # WiFi buffers, FreeRTOS heap, lwIP

    total_used_kb = fb_kb + model_kb + arena_kb + overhead_kb
    free_kb = args.psram_kb - total_used_kb

    print()
    print("=" * 60)
    print("  ESP32-S3 PSRAM Memory Budget")
    print("=" * 60)
    print(f"  PSRAM total:   {args.psram_kb:,} KB  ({args.psram_kb//1024} MB)")
    print(f"  Camera:        {width}×{height} {args.camera_format}, {args.fb_count} frame buffer(s)")
    print(f"  Model:         {model_kb} KB")
    print(f"  Tensor arena:  {arena_kb} KB {'(auto: 2× model)' if not args.arena_kb else ''}")
    print()
    print("  Allocation breakdown:")
    print_bar("Camera frame bufs", fb_kb,      args.psram_kb)
    print_bar("Model weights",     model_kb,   args.psram_kb)
    print_bar("Tensor arena",      arena_kb,   args.psram_kb)
    print_bar("System overhead",   overhead_kb, args.psram_kb)
    print()

    if free_kb >= 0:
        print(f"  ✓  FITS  —  {free_kb:.0f} KB free ({free_kb/args.psram_kb*100:.1f}%)")
    else:
        print(f"  ✗  DOES NOT FIT  —  {-free_kb:.0f} KB over budget")
        print()
        print("  Suggestions:")
        if args.camera_format != "grayscale":
            gray_kb = frame_buffer_bytes(width, height, "grayscale", args.fb_count) / 1024
            saved = fb_kb - gray_kb
            print(f"    • Switch to grayscale  → saves {saved:.0f} KB")
        if width > 96 or height > 96:
            small_kb = frame_buffer_bytes(96, 96, args.camera_format, args.fb_count) / 1024
            saved = fb_kb - small_kb
            print(f"    • Resize input to 96×96 → saves {saved:.0f} KB on camera side")
        if model_kb > 200:
            print(f"    • Use MobileNetV1 0.25× INT8 (~300 KB) instead of larger model")
        if args.fb_count > 1:
            saved = frame_buffer_bytes(width, height, args.camera_format, 1) / 1024
            print(f"    • Reduce to 1 frame buffer → saves {fb_kb - saved:.0f} KB")

    print("=" * 60)
    print()


if __name__ == "__main__":
    main()
