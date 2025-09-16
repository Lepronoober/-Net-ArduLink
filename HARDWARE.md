# µNet ArduLink Hardware Setup

This guide explains how to wire two Arduino boards for PWM-based communication
with a simple low-pass filter to stabilize the signal.

---

## Components Needed

- 2 Arduino boards (Uno, Nano, Mega, etc.)
- 2 resistors (10kΩ recommended)
- 2 capacitors (100nF recommended)
- Jumper wires
- Breadboard (optional)

---

## Wiring Overview

Arduino A Arduino B

PWM Pin 9 --------------> RX Pin 2
GND --------------> GND

- **PWM Pin** → output from the Arduino sending data
- **RX Pin** → input reading the PWM signal on the other Arduino
- **GND** → common ground between both boards

---

## Low-Pass Filter

To convert the PWM signal to a smoother analog voltage for better reading:

PWM Signal ---- R ----+----> RX Pin
|
C
|
GND


- **R** → resistor (10kΩ)
- **C** → capacitor (100nF)
- This forms a simple RC low-pass filter.

**How it works:**

- The PWM duty cycle produces an average voltage at the RX pin.
- The RC filter smooths the rapid switching to a quasi-analog voltage.
- This improves reliability of the `pulseIn()` measurement in the code.

---

## Tips

- Make sure both Arduinos share the **same ground**.
- Keep wire lengths short to reduce noise.
- Adjust R and C values if the PWM signal is too noisy.
- You can use multiple filters if you want cleaner signals for multiple channels.

---

## Optional: Debugging

- Connect an **oscilloscope** or **analog pin** to monitor the filtered PWM voltage.
- Adjust the RC values to get a stable voltage proportional to duty cycle.
- Verify `pulseIn()` reads values close to the expected PWM duty cycle.

---

## Summary

1. Connect PWM → RC → RX as shown.
2. Connect GNDs together.
3. Flash `unet_ardulink.ino` to both Arduinos.
4. Open Serial Monitor and test `dial` → `CONNECT`.
