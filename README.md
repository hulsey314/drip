# ESP32-S3 Stepper Controller

This project uses an **ESP32-S3 WROOM (Freenove board)** with a **TMC2209 stepper driver**, to test using a peristaltic pump to create drips with precise timing

---

## Features

* Adjust speed and distance for single "auto" drip, then press Auto button.  Ideally, this button will drop a single drip and prep the next drip.
* Manually move stepper motor forward or back to prep the next drip test (press Manual knob to run forward until released)

---

## Hardware Used

* [Freenove ESP32-S3 WROOM Board](https://github.com/Freenove/Freenove_ESP32_S3_WROOM)
* [TMC2209 Stepper Driver](https://www.trinamic.com/products/integrated-circuits/details/tmc2209/)
* NEMA 17 Stepper Motor (or similar)
* Rotary encoders (jog wheel with push button)
* Push button for motor control

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

1. Rotate or Hold the Manual Knob → Stepper moves relative to rotation - Prep a drop
2. Adjust Speed and Distance encoder knobs as desired
3. Press Auto button → Stepper runs selected Distance at selected Speed

Serial Monitor output will confirm encoder and button activity.

Values are stored in the ESP32's memory, so testing can be done without a computer and later plugged into a serial monitor to read Speed/Distance values.

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

![Setup](images/1-Setup.jpg)
![Layout](images/Layout.jpg)
![Stepper Motor Wiring](images/Stepper-Motor-Wiring.jpg)
Here is the wiring for the stepper motor.  These connections are more fragile than I expected...

---

## Next Steps

* 🔲 Try IR drip sensor to auto prep a drip
* 🔲 Test with smaller/cheaper servos

![Drip Detector - Back](images/Future--Drip-Detector--back.jpg)
![Drip Detector - Front (Closed)](images/Future--Drip-Detector--front--closed.jpg)
![Drip Detector - Front (Open)](images/Future--Drip-Detector--front--opened.jpg)
![Drip Detector - Front (Inside)](images/Future--Drip-Detector--front--inside.jpg)

---

## License

MIT License — free to use and modify.
