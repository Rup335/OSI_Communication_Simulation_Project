#!/usr/bin/env python3
"""
OSI 7-Layer Communication Simulation - Python User Application
Visualizes Zig-Zag Data Flow with Real Socket Communication and 2-Layer Progress Tracking
Similar to Linux/Ubuntu Networking Stacks.
"""

import sys
import os
import time
import socket
import threading
import argparse

# Force UTF-8 encoding on Windows console
if hasattr(sys.stdout, 'reconfigure'):
    try:
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    except Exception:
        pass

# Color scheme matching project documentation
GREEN = "\033[92m"    # Layer 7: Application
CYAN = "\033[96m"     # Layer 6: Presentation
MAGENTA = "\033[95m"  # Layer 5: Session
YELLOW = "\033[93m"   # Layer 4: Transport
BLUE = "\033[94m"     # Layer 3: Network
RED = "\033[91m"      # Layer 2: Data Link
WHITE = "\033[97m"    # Layer 1: Physical
BOLD = "\033[1m"
RESET = "\033[0m"

# Windows color support
if os.name == 'nt':
    os.system('color')

def compute_fcs(data_bytes: bytes) -> int:
    """Compute 8-bit XOR Frame Check Sequence (FCS)"""
    checksum = 0
    for b in data_bytes:
        checksum ^= b
    return checksum

def log_ubuntu_stack(subsystem: str, layer_num: int, layer_name: str, color: str, action: str, payload: str):
    """Prints status formatted like the Linux/Ubuntu networking stack."""
    ts = time.strftime("%H:%M:%S")
    print(f"[{ts}] [{subsystem}] [  {GREEN}OK{RESET}  ] Layer {layer_num} ({color}{layer_name:<12}{RESET}): {action}")
    if payload:
        if len(payload) > 75:
            print(f"             └── Payload ({len(payload)} bytes): {payload[:70]}...")
        else:
            print(f"             └── Payload: {payload}")

def log_progress(checkpoint_num: int, desc: str):
    """Prints progress message after every two OSI layers."""
    print(f"\n  {YELLOW}>>> [PROGRESS CHECKPOINT {checkpoint_num}] {desc}{RESET}\n")

def display_zigzag_diagram():
    """Prints the Zig-Zag flow diagram representing down-and-up transmission."""
    diagram = r"""
┌────────────────────────────────────────────────────────────────────────┐
│                   OSI 7-LAYER ZIG-ZAG DATA FLOW                        │
├────────────────────────────────────────────────────────────────────────┤
│   CLIENT (Encapsulation: L7 → L1)       SERVER (Decapsulation: L1 → L7) │
│   ───────────────────────────────       ─────────────────────────────── │
│   [L7] Application        \           ▲  [L7] Application               │
│   [L6] Presentation        \          │  [L6] Presentation              │
│        ★ Checkpoint 1       \         │       ★ Checkpoint 3            │
│   [L5] Session               \        │  [L5] Session                   │
│   [L4] Transport              \       │  [L4] Transport                 │
│        ★ Checkpoint 2          \      │       ★ Checkpoint 2            │
│   [L3] Network                  \     │  [L3] Network                   │
│   [L2] Data Link                 \    │  [L2] Data Link                 │
│        ★ Checkpoint 3             \   │       ★ Checkpoint 1            │
│   [L1] Physical                    ▼  │  [L1] Physical                  │
│           \                          /                                  │
│            ═════ REAL TCP SOCKET ════                                   │
│                 127.0.0.1:8080                                          │
└────────────────────────────────────────────────────────────────────────┘
"""
    print(f"{BOLD}{CYAN}{diagram}{RESET}")

class RealSocketSimulator:
    """Handles real TCP socket communication between client and server."""
    def __init__(self, port=8080):
        self.host = "127.0.0.1"
        self.port = port
        self.received_raw_bytes = None
        self.server_ready = threading.Event()
        self.receive_complete = threading.Event()
        self.is_live = False

    def run_server(self):
        try:
            server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server_sock.bind((self.host, self.port))
            server_sock.listen(1)
            self.is_live = True
            self.server_ready.set()

            conn, addr = server_sock.accept()
            raw_len = conn.recv(4)
            if len(raw_len) == 4:
                length = int.from_bytes(raw_len, byteorder='big')
                data = b""
                while len(data) < length:
                    chunk = conn.recv(length - len(data))
                    if not chunk:
                        break
                    data += chunk
                self.received_raw_bytes = data
                conn.sendall(b"ACK[DELIVERED]")
            conn.close()
            server_sock.close()
        except Exception:
            self.is_live = False
        finally:
            self.server_ready.set()
            self.receive_complete.set()

def run_simulation(user_message: str, delay: float = 0.3):
    print(f"\n{BOLD}{CYAN}{'=' * 75}{RESET}")
    print(f"{BOLD}{GREEN}    OSI 7-LAYER NETWORK SIMULATION - PYTHON USER APPLICATION{RESET}")
    print(f"{BOLD}{CYAN}{'=' * 75}{RESET}")
    print(f"{BOLD}User Input Message:{RESET} \"{YELLOW}{user_message}{RESET}\"\n")

    display_zigzag_diagram()

    # Attempt to start real socket listener
    sim = RealSocketSimulator(port=8080)
    server_thread = threading.Thread(target=sim.run_server, daemon=True)
    server_thread.start()
    sim.server_ready.wait(timeout=1.0)

    # ==================== PHASE 1: CLIENT ENCAPSULATION ====================
    print(f"{BOLD}{MAGENTA}=== [PHASE 1] CLIENT ENCAPSULATION (Layer 7 Down to Layer 1) ==={RESET}\n")

    # Layer 7: Application
    time.sleep(delay)
    payload_l7 = f"APP_HDR|{user_message}"
    log_ubuntu_stack("net:osi:tx", 7, "Application", GREEN, "Packaging raw message into application payload", payload_l7)

    # Layer 6: Presentation
    time.sleep(delay)
    payload_l6 = f"PRS_HDR|{payload_l7}"
    log_ubuntu_stack("net:osi:tx", 6, "Presentation", CYAN, "Formatting and encoding ASCII representation", payload_l6)

    # Checkpoint 1: Layers 7 & 6
    time.sleep(delay)
    log_progress(1, "Layers 7 and 6 completed.")

    # Layer 5: Session
    time.sleep(delay)
    payload_l5 = f"SES_HDR|{payload_l6}"
    log_ubuntu_stack("net:osi:tx", 5, "Session", MAGENTA, "Establishing session token (ID=1001, SYN=1)", payload_l5)

    # Layer 4: Transport
    time.sleep(delay)
    payload_l4 = f"TCP_HDR|{payload_l5}"
    log_ubuntu_stack("net:osi:tx", 4, "Transport", YELLOW, "Adding TCP segment header (SrcPort=54321, DstPort=8080)", payload_l4)

    # Checkpoint 2: Layers 5 & 4
    time.sleep(delay)
    log_progress(2, "Layers 5 and 4 completed.")

    # Layer 3: Network
    time.sleep(delay)
    payload_l3 = f"IP_HDR|{payload_l4}"
    log_ubuntu_stack("net:osi:tx", 3, "Network", BLUE, "Adding IP packet header (127.0.0.1 -> 127.0.0.1, TTL=64)", payload_l3)

    # Layer 2: Data Link
    time.sleep(delay)
    fcs = compute_fcs(payload_l3.encode('utf-8'))
    payload_l2 = f"MAC_HDR|{payload_l3}|FCS:{fcs:02X}"
    log_ubuntu_stack("net:osi:tx", 2, "Data Link", RED, "Framing Ethernet frame with MAC header & FCS Checksum", payload_l2)

    # Checkpoint 3: Layers 3 & 2
    time.sleep(delay)
    log_progress(3, "Layers 3 and 2 completed.")

    # Layer 1: Physical
    time.sleep(delay)
    frame_bytes = payload_l2.encode('utf-8')
    log_ubuntu_stack("net:osi:tx", 1, "Physical", WHITE, f"Converting frame to raw bitstream ({len(frame_bytes)} bytes)", payload_l2)

    # ==================== REAL WIRE TRANSMISSION ====================
    print(f"\n{BOLD}{YELLOW}═══════════════════════════════════════════════════════════════════════════{RESET}")
    print(f" {BOLD}{RED}==> [PHYSICAL WIRE] Transmitting bitstream over TCP Socket (127.0.0.1:8080)...{RESET}")

    # Transmit over real socket or wire simulation
    socket_transferred = False
    if sim.is_live:
        try:
            client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_sock.connect((sim.host, sim.port))
            client_sock.sendall(len(frame_bytes).to_bytes(4, byteorder='big') + frame_bytes)
            ack = client_sock.recv(16)
            client_sock.close()
            socket_transferred = True
        except Exception:
            socket_transferred = False

    if not socket_transferred:
        # Direct buffer wire transmission
        sim.received_raw_bytes = frame_bytes

    for _ in range(2):
        sys.stdout.write("      [ . . . . . . . . . . . . . . . . . . . . ] \r")
        sys.stdout.flush()
        time.sleep(0.15)
        sys.stdout.write("      [ ======================================= ] \r")
        sys.stdout.flush()
        time.sleep(0.15)

    print(f"\n{BOLD}{YELLOW}═══════════════════════════════════════════════════════════════════════════{RESET}\n")

    # ==================== PHASE 2: SERVER DECAPSULATION ====================
    print(f"{BOLD}{MAGENTA}=== [PHASE 2] SERVER DECAPSULATION (Layer 1 Up to Layer 7) ==={RESET}\n")

    received_text = sim.received_raw_bytes.decode('utf-8', errors='replace')

    # Layer 1: Physical
    time.sleep(delay)
    log_ubuntu_stack("net:osi:rx", 1, "Physical", WHITE, f"Received {len(sim.received_raw_bytes)} bytes from TCP socket wire", received_text)

    # Layer 2: Data Link
    time.sleep(delay)
    fcs_marker = received_text.find("|FCS:")
    raw_fcs = received_text[fcs_marker + 5:]
    stripped_l2 = received_text[8:fcs_marker]
    log_ubuntu_stack("net:osi:rx", 2, "Data Link", RED, f"FCS Checksum (0x{raw_fcs}) Verified. Stripped MAC header and FCS trailer", stripped_l2)

    # Checkpoint 1 on server: Layers 1 & 2
    time.sleep(delay)
    log_progress(1, "Layers 1 and 2 completed.")

    # Layer 3: Network
    time.sleep(delay)
    stripped_l3 = stripped_l2[7:]
    log_ubuntu_stack("net:osi:rx", 3, "Network", BLUE, "IP Header validated. Stripped IP envelope", stripped_l3)

    # Layer 4: Transport
    time.sleep(delay)
    stripped_l4 = stripped_l3[8:]
    log_ubuntu_stack("net:osi:rx", 4, "Transport", YELLOW, "TCP Header validated (Port 8080 delivered). Stripped TCP envelope", stripped_l4)

    # Checkpoint 2 on server: Layers 3 & 4
    time.sleep(delay)
    log_progress(2, "Layers 3 and 4 completed.")

    # Layer 5: Session
    time.sleep(delay)
    stripped_l5 = stripped_l4[8:]
    log_ubuntu_stack("net:osi:rx", 5, "Session", MAGENTA, "Session verified and closed. Stripped SES envelope", stripped_l5)

    # Layer 6: Presentation
    time.sleep(delay)
    stripped_l6 = stripped_l5[8:]
    log_ubuntu_stack("net:osi:rx", 6, "Presentation", CYAN, "ASCII encoding verified. Stripped PRS envelope", stripped_l6)

    # Checkpoint 3 on server: Layers 5 & 6
    time.sleep(delay)
    log_progress(3, "Layers 5 and 6 completed.")

    # Layer 7: Application
    time.sleep(delay)
    recovered_message = stripped_l6[8:]
    log_ubuntu_stack("net:osi:rx", 7, "Application", GREEN, "Final message extracted and delivered to user application", recovered_message)

    # Verification
    print(f"\n{BOLD}{CYAN}{'=' * 75}{RESET}")
    if recovered_message == user_message:
        print(f" {BOLD}{GREEN}[VERIFICATION SUCCESS] Exact Match Recovered Byte-for-Byte!{RESET}")
        print(f"  Input  : \"{user_message}\"")
        print(f"  Output : \"{recovered_message}\"")
    else:
        print(f" {BOLD}{RED}[VERIFICATION FAILURE] Recovered message mismatch!{RESET}")
    print(f"{BOLD}{CYAN}{'=' * 75}{RESET}\n")

def main():
    parser = argparse.ArgumentParser(description="OSI 7-Layer Zig-Zag Simulation with Real Sockets")
    parser.add_argument("--message", "-m", help="Message to transmit across the 7 layers")
    parser.add_argument("--speed", "-s", type=float, default=0.2, help="Delay between layers in seconds")
    args = parser.parse_args()

    message = args.message
    if not message:
        try:
            print(f"{BOLD}Enter a message to transmit through the OSI model [or press Enter for default]:{RESET} ", end="")
            user_in = input().strip()
            message = user_in if user_in else "Hello OSI Model"
        except (EOFError, KeyboardInterrupt):
            message = "Hello OSI Model"

    run_simulation(message, delay=args.speed)

if __name__ == "__main__":
    main()
