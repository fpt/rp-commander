#!/usr/bin/env python3
"""
commander.py — dump/load rp-commander profiles over CDC serial

Usage:
    python3 tools/commander.py dump [profiles.json]   # read from device
    python3 tools/commander.py load <profiles.json>   # write + save to device
    python3 tools/commander.py ports                  # list candidate ports

Requires: pip install pyserial
"""

import argparse
import json
import sys
import time
import serial
import serial.tools.list_ports


BAUD     = 115200
TIMEOUT  = 3.0   # seconds to wait for a response line


def find_port():
    """Return the first port that looks like an RP2040 CDC interface."""
    candidates = []
    for p in serial.tools.list_ports.comports():
        desc = (p.description or "").lower()
        mfr  = (p.manufacturer or "").lower()
        if "commander" in desc or "rp2040" in mfr or "raspberry" in mfr:
            candidates.append(p.device)
    if len(candidates) == 1:
        return candidates[0]
    if candidates:
        print("Multiple candidate ports found:", candidates, file=sys.stderr)
        print("Pass --port to select one.", file=sys.stderr)
        sys.exit(1)
    print("No rp-commander CDC port found. Use --port or 'ports' subcommand.",
          file=sys.stderr)
    sys.exit(1)


def open_port(port):
    return serial.Serial(port, BAUD, timeout=TIMEOUT)


def readline(ser):
    """Read one line, stripping CR/LF."""
    return ser.readline().decode("utf-8", errors="replace").rstrip("\r\n")


def expect_ok(ser, context=""):
    line = readline(ser)
    if line != "OK":
        raise RuntimeError(f"{context}: expected OK, got {line!r}")


def cmd_hello(ser):
    ser.write(b"HELLO\n")
    ser.flush()
    banner = readline(ser)   # "rp-commander 1.0"
    expect_ok(ser, "HELLO")
    return banner


def cmd_get(ser):
    ser.write(b"GET\n")
    ser.flush()
    json_line = readline(ser)
    expect_ok(ser, "GET")
    return json_line


def cmd_set(ser, json_str):
    payload = json_str.encode("utf-8")
    header  = f"SET {len(payload)}\n".encode()
    ser.write(header + payload)
    ser.flush()
    expect_ok(ser, "SET")


def cmd_save(ser):
    ser.write(b"SAVE\n")
    ser.flush()
    expect_ok(ser, "SAVE")


# ── Subcommands ───────────────────────────────────────────────────────────────

def do_ports(_args):
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("No serial ports found.")
        return
    for p in ports:
        print(f"  {p.device:20s}  {p.description}")


def do_dump(args):
    port = args.port or find_port()
    print(f"Connecting to {port} …")
    with open_port(port) as ser:
        time.sleep(0.1)          # let CDC settle
        ser.reset_input_buffer()
        banner = cmd_hello(ser)
        print(f"Device: {banner}")

        raw = cmd_get(ser)

    data = json.loads(raw)
    pretty = json.dumps(data, indent=2)

    out = args.file
    if out and out != "-":
        with open(out, "w") as f:
            f.write(pretty + "\n")
        print(f"Saved to {out}  ({len(raw)} bytes)")
    else:
        print(pretty)


def do_load(args):
    with open(args.file) as f:
        raw = f.read().strip()

    # Validate JSON before touching the device
    try:
        data = json.loads(raw)
    except json.JSONDecodeError as e:
        print(f"Invalid JSON: {e}", file=sys.stderr)
        sys.exit(1)

    # Re-serialise compact (no extra whitespace) — firmware expects compact JSON
    compact = json.dumps(data, separators=(",", ":"))

    port = args.port or find_port()
    print(f"Connecting to {port} …")
    with open_port(port) as ser:
        time.sleep(0.1)
        ser.reset_input_buffer()
        banner = cmd_hello(ser)
        print(f"Device: {banner}")

        print(f"Sending {len(compact)} bytes …")
        cmd_set(ser, compact)
        print("SET OK")

        cmd_save(ser)
        print("SAVE OK — profiles written to flash")


# ── Entry point ───────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="rp-commander profile tool")
    parser.add_argument("--port", "-p", help="Serial port (auto-detected if omitted)")
    sub = parser.add_subparsers(dest="cmd", required=True)

    p_ports = sub.add_parser("ports", help="List candidate serial ports")
    p_ports.set_defaults(func=do_ports)

    p_dump = sub.add_parser("dump", help="Read profiles from device")
    p_dump.add_argument("file", nargs="?", default="-",
                        help="Output file (default: stdout)")
    p_dump.set_defaults(func=do_dump)

    p_load = sub.add_parser("load", help="Write profiles to device and save to flash")
    p_load.add_argument("file", help="JSON file to upload")
    p_load.set_defaults(func=do_load)

    args = parser.parse_args()
    try:
        args.func(args)
    except RuntimeError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except KeyboardInterrupt:
        print("\nAborted.")
        sys.exit(1)


if __name__ == "__main__":
    main()
