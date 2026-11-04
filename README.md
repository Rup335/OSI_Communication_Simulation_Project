# OSI 7-Layer Network Model Simulation

A complete university implementation of the OSI 7-layer network model where **C-based client and server applications** implement full 7-layer encapsulation and decapsulation using **real TCP socket communication**, and a **Python user application** visualizes the **zig-zag data flow** with **progress tracking after every two OSI layers**, styled like Linux/Ubuntu networking stacks.

[![C Language](https://img.shields.io/badge/Language-C99-blue.svg)](https://en.wikipedia.org/wiki/C_%28programming_language%29)
[![OSI Model](https://img.shields.io/badge/Architecture-OSI%207--Layers-orange.svg)](file:///OSI_Model.md)
[![Sockets](https://img.shields.io/badge/Networking-Real%20TCP%20Sockets-green.svg)](https://en.wikipedia.org/wiki/Network_socket)
[![Python](https://img.shields.io/badge/User%20App-Python%203.x-yellow.svg)](https://python.org)
[![Stack Style](https://img.shields.io/badge/Log%20Style-Linux%2FUbuntu-red.svg)](https://ubuntu.com)

---

## 📋 Table of Contents
* [Overview](#overview)
* [Architecture & Zig-Zag Data Flow](#architecture--zig-zag-data-flow)
* [OSI Layer Functions & Protocol](#osi-layer-functions--protocol)
* [Two-Layer Progress Checkpoints](#two-layer-progress-checkpoints)
* [Project Structure](#project-structure)
* [How to Run the Python Visualizer Application](#how-to-run-the-python-visualizer-application)
* [How to Build & Run C Client / Server](#how-to-build--run-c-client--server)
* [Linux & Ubuntu Compatibility](#linux--ubuntu-compatibility)
* [Screenshots Guide for Assignment Submission](#screenshots-guide-for-assignment-submission)

---

## 🚀 Overview

This simulation demonstrates:

1. **C Client Application**: Takes user input, runs Layer 7 down to Layer 1 encapsulation with 2-layer progress checkpoints, and transmits framed data over a real TCP socket.
2. **C Server Application**: Listens on `127.0.0.1:8080`, receives the raw stream from the socket, runs Layer 1 up to Layer 7 decapsulation with 2-layer progress checkpoints, and validates the exact original message.
3. **Python User Application**: Interactive application that runs the complete zig-zag simulation over real sockets, tracking data flow step-by-step with Linux/Ubuntu networking stack output.
4. **Linux/Ubuntu Networking Stack Format**: Structured log banners (`[net:osi:tx] [ OK ] ...`) with FCS checksum verification and lossless payload recovery.

---

## 🏗️ Architecture & Zig-Zag Data Flow

```text
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
```

---

## 📦 OSI Layer Functions & Protocol

Each layer encapsulates the payload with a standardized header prefix, and strips it on decapsulation:

| Layer | Send Function | Receive Function | Protocol Header | Responsibility |
| :--- | :--- | :--- | :--- | :--- |
| **7. Application** | `app_send()` | `app_recv()` | `APP_HDR\|data` | User message generation & delivery |
| **6. Presentation**| `present_send()` | `present_recv()` | `PRS_HDR\|...` | ASCII encoding & data formatting |
| **5. Session** | `session_send()` | `session_recv()` | `SES_HDR\|...` | Session tokens & synchronization |
| **4. Transport** | `transport_send()`| `transport_recv()`| `TCP_HDR\|...` | Port numbers (54321 → 8080) & Seq |
| **3. Network** | `network_send()` | `network_recv()` | `IP_HDR\|...` | IP routing (127.0.0.1) & TTL |
| **2. Data Link** | `datalink_send()` | `datalink_recv()` | `MAC_HDR\|...\|FCS:XX`| MAC framing & Frame Check Checksum |
| **1. Physical** | `physical_send()` | `physical_recv()` | `[4-Byte Len] + Bits` | Real TCP socket byte transmission |

---

## 🚩 Two-Layer Progress Checkpoints

As required, progress checkpoints are printed after every two OSI layers:

### Transmission Side (Downward: L7 → L1)
- **Checkpoint 1**: Printed after completing **Layers 7 & 6** (`Application` & `Presentation`).
- **Checkpoint 2**: Printed after completing **Layers 5 & 4** (`Session` & `Transport`).
- **Checkpoint 3**: Printed after completing **Layers 3 & 2** (`Network` & `Data Link`).
- **Layer 1**: Transmits framed bitstream across the real socket.

### Reception Side (Upward: L1 → L7)
- **Checkpoint 1**: Printed after completing **Layers 1 & 2** (`Physical` & `Data Link` - FCS verified).
- **Checkpoint 2**: Printed after completing **Layers 3 & 4** (`Network` & `Transport` - Port verified).
- **Checkpoint 3**: Printed after completing **Layers 5 & 6** (`Session` & `Presentation` - Session closed).
- **Layer 7**: Recovers and verifies exact original message byte-for-byte.

---

## 📁 Project Structure

```text
OSI_Project_Git/
├── visualizer.py                  # Python User Application (Zig-zag visualization & sockets)
├── README.md                      # Comprehensive documentation
├── OSI_Model.md                   # Full OSI theoretical study notes
├── LICENSE                        # MIT License
├── build.bat                      # 1-Click Windows build script (MinGW GCC)
├── Makefile                       # Cross-platform Makefile (Windows / Linux)
│
├── src/                           # Canonical C source code
│   ├── osi_common.h               # Cross-platform socket layer (Winsock2 & POSIX)
│   ├── layers.h                   # OSI layer function declarations
│   ├── layers.c                   # Encapsulation, decapsulation, checkpoints & FCS
│   ├── client.c                   # Standalone C client application (real socket sender)
│   └── server.c                   # Standalone C server application (real socket listener)
│
├── python/                        # Python module directory
│   └── visualizer.py              # Visualizer application
│
└── OSI-communication/             # Synchronized test harness
    ├── layers.h
    ├── layers.c
    ├── main.c
    └── Makefile
```

---

## 🐍 How to Run the Python Visualizer Application

The Python user application is the primary visualizer for the assignment:

```bash
# Run with interactive prompt:
python visualizer.py

# Or run with a custom message:
python visualizer.py --message "Hello from OSI layers!"

# Adjust animation speed:
python visualizer.py --message "Networking Simulation" --speed 0.3
```

---

## ⚙️ How to Build & Run C Client / Server

### On Windows (MinGW GCC)
1. **Compile**:
   Double-click `build.bat` or run:
   ```cmd
   mkdir bin
   gcc -Wall -O2 src/server.c src/layers.c -o bin/server.exe -lws2_32
   gcc -Wall -O2 src/client.c src/layers.c -o bin/client.exe -lws2_32
   ```

2. **Run Server** (in first terminal):
   ```cmd
   .\bin\server.exe
   ```

3. **Run Client** (in second terminal):
   ```cmd
   .\bin\client.exe "Hello OSI Model"
   ```

---

## 🐧 Linux & Ubuntu Compatibility

The C source code is cross-platform. On Linux or Ubuntu:
```bash
# Compile
gcc -Wall -O2 src/server.c src/layers.c -o server
gcc -Wall -O2 src/client.c src/layers.c -o client

# Run Server
./server &

# Run Client
./client "Hello from Linux OSI stack!"

# Run Python Visualizer
python3 visualizer.py --message "Ubuntu Networking Stack"
```

---

## 📸 Screenshots Guide for Assignment Submission

1. **Python Zig-Zag Flow**: Run `python visualizer.py` and take a screenshot of the colored diagram and Linux-style log output.
2. **Progress Checkpoints**: Highlight **Checkpoint 1, 2, and 3** in your screenshot.
3. **C Real Socket Run**: Take a screenshot with `server.exe` on the left and `client.exe` on the right showing the transmission over `127.0.0.1:8080` and the `[VERIFICATION SUCCESS]` match.
