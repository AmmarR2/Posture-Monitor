# Posture Monitor

A wearable posture-monitor prototype built with an **Arduino Uno**, **MPU-6050 motion sensor**, and **vibration motor**. The device measures upper-body tilt and gives physical feedback when the user remains in a slouched position.

## Current Status

**Working prototype.**

The current version can:

- Read posture angle from the MPU-6050
- Classify posture as upright or slouching
- Wait 3 seconds before issuing an alert
- Activate a vibration motor while slouching
- Stop the motor immediately when posture returns upright
- Run independently after the program has been uploaded to the Arduino

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
          | calculates angle
          | checks -115° threshold
          | waits 3 seconds
          v
       D9 signal
          |
       1k resistor
          |
     S8050 transistor
          |
          v
   Vibration motor
```

The MPU-6050 sends acceleration measurements to the Arduino over I2C. The Arduino calculates a tilt angle using the X and Z acceleration axes. If the angle remains below the slouch threshold for at least three seconds, Digital Pin 9 activates the transistor controlling the vibration motor.

Returning to an upright position immediately turns the motor off and resets the timer.

## Hardware

- Arduino Uno R3
- GY-521 / MPU-6050 accelerometer and gyroscope module
- Breadboard
- Jumper wires
- Small vibration motor
- S8050 NPN transistor
- 1kΩ resistor
- 1N4007 flyback diode
- USB power source / portable power solution

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

const int MPU = 0x68;
const int motorPin = 9;

unsigned long slouchStart = 0;
bool timingSlouch = false;

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

  Serial.print("Angle: ");
  Serial.print(angle);
  Serial.print(" | ");

  if (angle < -115) {
    Serial.print("SLOUCHING");

    if (!timingSlouch) {
      slouchStart = millis();
      timingSlouch = true;
    }

    if (millis() - slouchStart >= 3000) {
      digitalWrite(motorPin, HIGH);
      Serial.print(" - VIBRATING");
    }
  } else {
    Serial.print("UPRIGHT");
    timingSlouch = false;
    digitalWrite(motorPin, LOW);
  }

  Serial.println();
  delay(100);
}
```

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

During development, the Arduino can be powered through USB from a computer. After firmware is uploaded, the Arduino stores the program and does not require a computer to run.

For a portable demonstration, the easiest approach is a **5V USB power bank connected directly to the Arduino Uno USB port**. This allows the existing circuit to remain unchanged.

A 7.4V (2S) LiPo is also being evaluated as an alternative portable power source. A 2S LiPo must **never be connected directly to the Arduino 5V pin or USB power line**. The battery connector, polarity, and power arrangement must be verified before using it.

## Prototype Goal

The immediate goal is a portable proof-of-concept that can be worn on the chest for a demonstration:

```text
     Chest / lanyard
           |
      MPU-6050
           |
      Arduino Uno
           |
   vibration feedback
           |
   portable battery
```

The first wearable version prioritizes demonstrating that the sensing and feedback system works rather than minimizing device size.

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
- [ ] Add portable battery power
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
- Threshold-based detection
- Non-blocking timing with `millis()`
- NPN transistor switching
- Inductive-load flyback protection
- Prototype power distribution
- Wearable electronics design
- Iterative calibration and testing

## Future Improvements

Once the Arduino prototype is demonstrated successfully, possible improvements include a smaller microcontroller, rechargeable integrated battery, custom PCB/perfboard, automatic posture calibration, improved filtering, data logging, Bluetooth connectivity, and a purpose-built wearable enclosure.
