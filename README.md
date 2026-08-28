# Posture Monitor

A wearable posture-monitor prototype built with an **Arduino Uno**, **MPU-6050 motion sensor**, and **vibration motor**. The device measures upper-body tilt, gives physical feedback when the user remains in a slouched position, and tracks posture statistics over time.

## Current Status

**Working prototype with persistent data logging.**

The current version can:

- Read posture angle from the MPU-6050
- Classify posture as upright or slouching
- Wait 3 seconds before issuing an alert
- Activate a vibration motor while slouching
- Stop the motor immediately when posture returns upright
- Track upright time and slouching time
- Calculate upright and slouching percentages
- Count slouch events
- Track total recorded session time
- Save statistics to Arduino EEPROM every 10 seconds
- Reload saved statistics after power is disconnected and restored
- Run independently from a 5V USB power bank after the program is uploaded

The current measured threshold is approximately **-115°**:

- Angle greater than or equal to -115° → upright
- Angle below -115° → slouching

This threshold depends on how the sensor is mounted and can be adjusted in firmware.

## How It Works

```text
                 MPU-6050
              posture sensor
                    |
                    | I2C
                    v
               Arduino Uno
                    |
        +-----------+-----------+
        |                       |
        v                       v
 posture detection         posture statistics
        |                       |
        v                       v
 3-second slouch          EEPROM storage
     timer                  every 10 sec
        |
        v
 Arduino D9
        |
    1k resistor
        |
 S8050 transistor
        |
        v
 vibration motor
```

The MPU-6050 sends acceleration measurements to the Arduino over I2C. The Arduino calculates a tilt angle using the X and Z acceleration axes. If the angle remains below the slouch threshold for at least three seconds, Digital Pin 9 activates the transistor controlling the vibration motor.

At the same time, the Arduino records how long the user is upright or slouching and saves those totals to EEPROM. EEPROM keeps the statistics even when the Arduino is unplugged from the power bank.

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

### Planned cleanup

The working full-size breadboard prototype can later be transferred to a **400-point half-size breadboard** with shorter jumper wires to make the assembly smaller and cleaner.

## Wiring

### MPU-6050

| MPU-6050 | Arduino Uno |
|---|---|
| VCC | 5V power rail |
| GND | GND rail |
| SDA | A4 |
| SCL | A5 |

### Breadboard power

| Arduino | Breadboard |
|---|---|
| 5V | Positive (+) rail |
| GND | Negative (-) rail |

### Vibration motor driver

The S8050 transistor is used so the Arduino GPIO does **not** directly power the motor.

```text
Power + ---- Motor ---- S8050 Collector
                         |
Arduino D9 -- 1kΩ -- Base
                         |
GND ----------------- Emitter
```

A **1N4007 diode** is connected across the motor as a flyback diode to protect the electronics from the voltage spike produced when the motor switches off.

Current breadboard implementation:

- S8050 emitter → GND
- S8050 base → 1kΩ resistor → Arduino D9
- S8050 collector → motor negative
- Motor positive → positive power rail
- 1N4007 striped end → positive motor/power side
- 1N4007 non-striped end → motor negative / transistor collector side

> S8050 pinouts can vary by manufacturer. Verify the transistor's datasheet before rebuilding the circuit.

## Firmware

```cpp
#include <Wire.h>
#include <math.h>
#include <EEPROM.h>

const int MPU = 0x68;
const int motorPin = 9;
const float slouchThreshold = -115.0;

unsigned long slouchStart = 0;
bool timingSlouch = false;
bool inSlouch = false;

unsigned long uprightTime = 0;
unsigned long slouchTime = 0;
unsigned long slouchEvents = 0;

unsigned long lastUpdate = 0;
unsigned long lastSave = 0;

struct PostureData {
  unsigned long uprightTime;
  unsigned long slouchTime;
  unsigned long slouchEvents;
};

PostureData savedData;

void saveData() {
  savedData.uprightTime = uprightTime;
  savedData.slouchTime = slouchTime;
  savedData.slouchEvents = slouchEvents;

  EEPROM.put(0, savedData);
}

void setup() {
  Wire.begin();
  Serial.begin(9600);

  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);

  // Wake MPU-6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Load previously saved statistics
  EEPROM.get(0, savedData);

  // Handle completely uninitialized EEPROM
  if (savedData.uprightTime == 0xFFFFFFFF) {
    uprightTime = 0;
    slouchTime = 0;
    slouchEvents = 0;
  } else {
    uprightTime = savedData.uprightTime;
    slouchTime = savedData.slouchTime;
    slouchEvents = savedData.slouchEvents;
  }

  lastUpdate = millis();

  Serial.println("POSTURE MONITOR STARTED");
  Serial.println("-----------------------");
}

void loop() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);

  int16_t ax = Wire.read() << 8 | Wire.read();
  int16_t ay = Wire.read() << 8 | Wire.read();
  int16_t az = Wire.read() << 8 | Wire.read();

  float angle = atan2(ax, az) * 180.0 / PI;

  unsigned long now = millis();
  unsigned long elapsed = now - lastUpdate;
  lastUpdate = now;

  Serial.print("Angle: ");
  Serial.print(angle);
  Serial.print(" | ");

  if (angle < slouchThreshold) {
    Serial.print("SLOUCHING");
    slouchTime += elapsed;

    if (!inSlouch) {
      slouchEvents++;
      inSlouch = true;
      slouchStart = millis();
      timingSlouch = true;
    }

    if (timingSlouch && millis() - slouchStart >= 3000) {
      digitalWrite(motorPin, HIGH);
      Serial.print(" - VIBRATING");
    }
  } else {
    Serial.print("UPRIGHT");
    uprightTime += elapsed;

    inSlouch = false;
    timingSlouch = false;
    digitalWrite(motorPin, LOW);
  }

  Serial.println();

  // Save posture statistics every 10 seconds
  if (millis() - lastSave >= 10000) {
    saveData();
    lastSave = millis();
    Serial.println("[DATA SAVED]");
  }

  // Print a summary every 5 seconds
  static unsigned long lastStats = 0;

  if (millis() - lastStats >= 5000) {
    unsigned long total = uprightTime + slouchTime;

    float uprightPercent = 0;
    float slouchPercent = 0;

    if (total > 0) {
      uprightPercent = (uprightTime * 100.0) / total;
      slouchPercent = (slouchTime * 100.0) / total;
    }

    Serial.println();
    Serial.println("===== POSTURE STATS =====");

    Serial.print("Upright: ");
    Serial.print(uprightPercent, 1);
    Serial.println("%");

    Serial.print("Slouching: ");
    Serial.print(slouchPercent, 1);
    Serial.println("%");

    Serial.print("Slouch Events: ");
    Serial.println(slouchEvents);

    Serial.print("Total Recorded: ");
    Serial.print(total / 60000.0, 1);
    Serial.println(" minutes");

    Serial.println("=========================");
    Serial.println();

    lastStats = millis();
  }

  delay(100);
}
```

## Posture Statistics

The Arduino now calculates a running session summary while the device is powered.

Example Serial Monitor output:

```text
===== POSTURE STATS =====
Upright: 76.4%
Slouching: 23.6%
Slouch Events: 4
Total Recorded: 2.5 minutes
=========================
```

The same summary can be viewed while testing directly from a computer over USB.

### Persistent storage

Statistics are saved to the Arduino Uno's built-in EEPROM every **10 seconds**. This means the Arduino can be:

```text
Power bank -> Arduino records posture
                 |
                 v
             EEPROM save
                 |
        power disconnected
                 |
                 v
          data stays saved
                 |
      Arduino connected to PC
                 |
                 v
      Serial Monitor shows data
```

Because data is saved every 10 seconds, disconnecting power may lose up to approximately the final 10 seconds of measurements.

## Detection Logic

Example readings from the current sensor position:

```text
-100°  -> UPRIGHT
-110°  -> UPRIGHT
-120°  -> SLOUCHING
-130°  -> SLOUCHING
```

The **-115° threshold is not universal**. Moving or rotating the MPU-6050 changes the readings, so the threshold should be measured again whenever the sensor mounting position changes.

## Power

During development, the Arduino can be powered through USB from a computer. This allows the posture statistics to be tested directly in the Arduino Serial Monitor.

For the portable demonstration, the device will use a **5V USB power bank connected directly to the Arduino Uno USB port**. No wiring changes are required when switching between computer USB power and power-bank USB power.

After the demonstration, the power bank can be disconnected and the Arduino plugged back into the computer. The saved EEPROM statistics are then restored automatically.

## Prototype Goal

The immediate goal is a portable proof-of-concept that can be worn on the chest for a demonstration:

```text
       Chest / lanyard
             |
        MPU-6050
             |
        Arduino Uno
         /       \
        /         \
 vibration      EEPROM
 feedback       statistics
        |
        v
   USB power bank
```

The first wearable version prioritizes demonstrating that posture sensing, vibration feedback, and session tracking all work reliably rather than minimizing device size.

## Development Progress

- [x] Connect MPU-6050 to Arduino
- [x] Read accelerometer data
- [x] Calculate posture angle
- [x] Determine upright/slouch measurements
- [x] Implement adjustable slouch threshold
- [x] Require continuous slouching before alert
- [x] Build transistor motor driver
- [x] Add flyback diode
- [x] Activate vibration motor from firmware
- [x] Stop vibration when posture is corrected
- [x] Working integrated prototype
- [x] Track upright and slouching time
- [x] Calculate posture percentages
- [x] Count slouch events
- [x] Save statistics to EEPROM
- [x] Restore statistics after power loss
- [ ] Test portable USB power bank
- [ ] Transfer circuit to smaller breadboard
- [ ] Shorten and organize wiring
- [ ] Mount sensor consistently against chest
- [ ] Build lanyard/enclosure
- [ ] Test complete portable demonstration

## Engineering Concepts

This project demonstrates:

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
- Session data persistence across power loss
- Prototype power distribution
- Wearable electronics design
- Iterative calibration and testing

## Future Improvements

Possible future improvements include average slouch duration, a reset-session command, improved sensor filtering, automatic posture calibration, data logging to an external device, Bluetooth/Wi-Fi connectivity, a smaller microcontroller, rechargeable integrated battery, custom PCB/perfboard, and a purpose-built wearable enclosure.
