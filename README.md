# ESP32-S3 Stepper Controller

This project uses an **ESP32-S3 WROOM (Freenove board)** with a **TMC2209 stepper driver**, rotary encoder (jog wheel), and buttons to control a stepper motor. It also integrates **Bluetooth Low Energy (BLE)** for future expansion.

---

## Features

* Control stepper motor direction and speed with a jog wheel (rotary encoder)
* Press jog encoder for **continuous forward run** until released
* Dedicated button for additional stepper actions
* BLE server code included for future remote control and monitoring
* Designed for **Freenove ESP32-S3 board** with pins optimized for working peripherals

---

## Hardware Used

* [Freenove ESP32-S3 WROOM Board](https://github.com/Freenove/Freenove_ESP32_S3_WROOM)
* [TMC2209 Stepper Driver](https://www.trinamic.com/products/integrated-circuits/details/tmc2209/)
* NEMA 17 Stepper Motor (or similar)
* Rotary encoder (jog wheel with push button)
* Extra push button for motor control

---

## Wiring Overview

| Component          | ESP32-S3 Pin        |
| ------------------ | ------------------- |
| Stepper DIR        | GPIO 12             |
| Stepper STEP       | GPIO 14             |
| Stepper ENABLE     | GPIO 27             |
| Jog Encoder A      | GPIO 4              |
| Jog Encoder B      | GPIO 5              |
| Jog Encoder Button | GPIO 15             |
| Extra Button       | GPIO 21 (suggested) |

*(You can adjust pins in `main.cpp` as needed.)*

---

## Usage

1. Rotate the jog encoder → Stepper moves relative to rotation
2. Press jog encoder → Stepper runs continuously forward until released
3. Press extra button → Custom stepper action (e.g., reverse jog or home)

Serial Monitor output will confirm encoder and button activity.

---

## Installation

### Prerequisites

* [PlatformIO](https://platformio.org/) inside VS Code
* Git installed

### Clone & Build

```bash
git clone https://github.com/YOUR-USERNAME/esp32-s3-stepper-controller.git
cd esp32-s3-stepper-controller
platformio run --target upload
```

### Serial Monitor

```bash
platformio device monitor
```

---

## Images

Add images of your setup here (place inside the `images/` folder and link them):

![ESP32-S3 Board](images/esp32s3.jpg)
![Stepper Motor](images/stepper.jpg)
![Wiring Setup](images/wiring.jpg)

---

## Next Steps

* ✅ Add Bluetooth support for wireless jog and button control
* ✅ Expand UI with a mobile app or BLE MIDI commands
* 🔲 Add OLED or e-ink screen for real-time feedback
* 🔲 Explore Wi-Fi control & MQTT integration
* 🔲 Develop PCB version for easier wiring

![Concept UI](images/concept_ui.jpg)

---

## License

MIT License — free to use and modify.
