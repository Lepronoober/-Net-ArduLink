# µNet ArduLink

µNet ArduLink is a lightweight experimental network protocol for Arduino boards,
allowing point-to-point communication over **PWM-based signaling**.

It provides:
- A handshake system inspired by dial-up & DHCP (dynamic ID assignment).
- Framed communication with **ID, SEQ, LEN, DATA, CRC**.
- A simple **chat-like messaging demo**.
- Extendable design for multi-Arduino networks.

---

## Features
- Dynamic ID assignment (DHCP-like).
- Connection establishment (`DIAL` / `CONNECT` / `BYE`).
- Message framing with CRC validation.
- ACK mechanism for reliability.
- PWM-based data exchange between boards.

---

## Protocol Overview

Each frame is structured as:

[ID] [SEQ] [LEN] [DATA...] [CRC]

- **ID** → Sender ID (assigned dynamically).
- **SEQ** → Sequence number.
- **LEN** → Length of payload.
- **DATA** → Payload bytes.
- **CRC** → Checksum of the frame.

---

## How to Use

1. Flash `src/unet_ardulink.ino` to **two Arduino boards**.
2. Connect:
   - `PWM out (pin 9)` → `RX in (pin 2)` of the other board.
   - Common GND.
3. Open **Serial Monitor** at `9600 baud`.
4. On one board, type: dial
→ It will initiate a connection.
5. Exchange messages once connected.

---

## Roadmap
- [ ] Improve timing & PWM precision.
- [ ] Multi-board arbitration.
- [ ] Implement packet retransmission.
- [ ] Add higher-level protocols (chat, file transfer).

---

## 📄 License
MIT License – do whatever you want with proper attribution.
