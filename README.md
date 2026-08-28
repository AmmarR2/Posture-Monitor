# Posture Monitor (Arduino + MPU-6050 + Vibration Feedback)

A wearable posture monitoring system that measures upper-body tilt using an Arduino Uno and MPU-6050 motion sensor. When the user remains slouched for 3 seconds, the Arduino activates a vibration motor as a physical reminder to correct posture. The device also tracks posture statistics and saves them to EEPROM so the data remains stored after power is disconnected.

---

## Features

- Real-time posture angle monitoring
- Upright and slouching detection
- 3-second slouch delay before vibration alert
- Automatic vibration feedback
- Immediate vibration shutoff when posture is corrected
- Upright time tracking
- Slouching time tracking
- Upright and slouching percentage calculations
- Slouch event counter
- Total recorded session time
- EEPROM data storage every 10 seconds
- Saved statistics restored after power loss
- Portable operation using a 5V USB power bank

---

## How It Works

1. The MPU-6050 measures acceleration and body orientation.
2. The Arduino reads the sensor through I2C communication.
3. The Arduino calculates a posture angle using the X and Z acceleration axes.
4. The angle is compared with the calibrated slouch threshold.
5. If slouching continues for at least 3 seconds, Arduino pin D9 activates the S8050 transistor.
6. The transistor switches on the vibration motor to alert the user.
7. The motor stops immediately when upright posture is restored.
8. Upright time, slouching time, and slouch events are tracked continuously.
9. Statistics are saved to EEPROM every 10 seconds so they remain available after power is removed.

The current calibrated threshold is approximately **-115°**. This value depends on how the MPU-6050 is mounted and can be changed in the firmware.

---

## Hardware

- Arduino Uno R3
- GY-521 / MPU-6050 accelerometer and gyroscope module
- Breadboard
- Jumper wires
- Small vibration motor
- S8050 NPN transistor
- 1kΩ resistor
- 1N4007 flyback diode
- 5V USB power bank
- USB cable

---

# Wiring Diagram
<img width="1536" height="1024" alt="image" src="https://github.com/user-attachments/assets/de30b2d2-35e1-4049-ad0a-110490066a67" />

---

# Posture Device Prototype Preview

<img width="3210" height="2182" alt="image" src="https://github.com/user-attachments/assets/7d3773d1-6b60-4c72-af6a-2b0a7f869efe" />

---

# Software Used

- Arduino IDE
- Arduino C/C++
- Wire library
- EEPROM library
- Arduino Serial Monitor

---

# Setup Instructions

## 1. Build the Circuit

Build the circuit using the wiring diagram image above.

## 2. Connect the Arduino

Connect the Arduino Uno to the computer using USB.

## 3. Upload the Arduino Code

Open the posture monitor `.ino` file in Arduino IDE, select the correct Arduino Uno board and COM port, and upload the program.

## 4. Open Serial Monitor

Open the Arduino Serial Monitor and set the baud rate to:

```text
9600
```

The Arduino will begin displaying the current posture angle and whether the user is upright or slouching.

Example:

```text
Angle: -100.4 | UPRIGHT
Angle: -121.7 | SLOUCHING
Angle: -125.2 | SLOUCHING - VIBRATING
```

## 5. Calibrate the Posture Threshold

The current firmware uses:

```cpp
const float slouchThreshold = -115.0;
```

Example readings from the current sensor position:

```text
-100°  -> UPRIGHT
-110°  -> UPRIGHT
-120°  -> SLOUCHING
-130°  -> SLOUCHING
```

The threshold is not universal. If the sensor is moved or rotated, measure the upright and slouched angles again and adjust the value in the firmware.

## 6. Use Portable Power

After the program is uploaded, disconnect the Arduino from the computer and connect a **5V USB power bank directly to the Arduino Uno USB port**.

The device can then run independently without a computer.

---

# Data Tracking

The Arduino continuously tracks:

- Upright percentage
- Slouching percentage
- Number of slouch events
- Total recorded time

Example output:

```text
===== POSTURE STATS =====
Upright: 76.4%
Slouching: 23.6%
Slouch Events: 4
Total Recorded: 2.5 minutes
=========================
```

Statistics are saved to the Arduino Uno's EEPROM every **10 seconds**. When the Arduino is powered off and later reconnected, the stored totals are automatically restored.

Because the data is saved every 10 seconds, disconnecting power may lose approximately the final 10 seconds of measurements.

---

# Engineering Concepts

- Embedded systems programming
- I2C communication
- Accelerometer-based orientation sensing
- Real-time sensor processing
- Threshold-based posture detection
- Non-blocking timing with `millis()`
- NPN transistor switching
- Inductive-load flyback protection
- EEPROM non-volatile data storage
- Running-time statistics and percentages
- Portable power distribution
- Wearable electronics prototyping
- Sensor calibration and testing

---

# Future Improvements

- Transfer the circuit to a smaller breadboard
- Shorten and organize jumper wires
- Build a wearable lanyard or enclosure
- Improve sensor filtering
- Add automatic posture calibration
- Track average slouch duration
- Add a reset-session command
- Add Bluetooth or Wi-Fi connectivity
- Send posture data to a computer or phone dashboard
- Replace the Arduino Uno with a smaller microcontroller
- Build a custom PCB or perfboard version
- Add an integrated rechargeable battery
