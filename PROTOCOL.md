# µNet ArduLink Protocol Specification

## Overview
µNet ArduLink is a lightweight PWM-based protocol for Arduino-to-Arduino communication. 
It supports peer-to-peer messaging with dynamic ID assignment.

---

## Frame Structure

Each message frame contains:

| Field | Size | Description |
|-------|------|-------------|
| ID    | 1 byte | Sender ID (assigned dynamically via DHCP-like handshake) |
| SEQ   | 1 byte | Sequence number (incremented each message) |
| LEN   | 1 byte | Length of DATA in bytes |
| DATA  | LEN bytes | Payload (ASCII or binary data) |
| CRC   | 1 byte | Simple checksum: sum(ID + SEQ + LEN + DATA) mod 256 |

---

## Message Types

- `DIAL` → Initiates a connection.
- `CONNECT` → Response to `DIAL`, confirms connection.
- `ASSIGN:<ID>` → Server assigns dynamic ID.
- `BYE` → Terminates connection.
- Arbitrary messages → Any string/byte array after connection.
- ACK → Acknowledgement of received messages.

---

## Communication Flow

1. Client with ID=0 sends `DIAL`.
2. Server receives `DIAL`, allocates a free ID, responds with `ASSIGN:<ID>` and `CONNECT`.
3. Client updates its ID and confirms connection established.
4. Data messages can now be sent with framing `[ID][SEQ][LEN][DATA][CRC]`.
5. ACK messages confirm reception of each frame.
6. `BYE` terminates the session and releases IDs on the server.

---

## Timing

- PWM symbol duration: 20 ms
- Maximum message length: limited by buffer memory (~64 bytes per message recommended)
