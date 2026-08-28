# Posture Monitor

A wearable/desk posture-monitor prototype built around an Arduino Uno and MPU-6050 motion sensor.

## Goal

Detect when the user starts slouching and provide a simple reminder using vibration and/or an LED.

The first version is intentionally simple so the sensing logic, calibration, electronics, and feedback system can be tested before making the device smaller or wearable.

## How It Works

```text
[MPU-6050 motion sensor]
          |
          | orientation / tilt data
          v
      [Arduino Uno]
          |
          +--> posture calculation
          +--> slouch threshold
          |
          v
 [LED / vibration motor]
```

The MPU-6050 contains an accelerometer and gyroscope. The Arduino reads the sensor, estimates the user's orientation, compares it with a calibrated "good posture" position, and triggers feedback if poor posture continues long enough.

## Prototype Hardware

Planned prototype components:

- Arduino Uno R3
- GY-521 MPU-6050 accelerometer/gyroscope module
- Breadboard
- Jumper wires
- LED
- Current-limiting resistor
- Small vibration motor
- NPN transistor for driving the motor
- Flyback diode for the motor
- External battery/power solution later in development

## Repository Structure

```text
Posture-Monitor/
├── firmware/            # Arduino firmware
├── docs/                # Wiring, parts, design notes, experiments
├── README.md
└── .gitignore
```

## Development Milestones

- [ ] Read raw MPU-6050 accelerometer data
- [ ] Read gyroscope data
- [ ] Calculate useful tilt/orientation values
- [ ] Create a calibration routine for good posture
- [ ] Define a slouch threshold
- [ ] Require poor posture for a short time before alerting
- [ ] Trigger an LED warning
- [ ] Drive a vibration motor through a transistor
- [ ] Tune sensitivity and reduce false alerts
- [ ] Move from breadboard prototype to compact hardware
- [ ] Design a wearable or desk-mounted enclosure

## Current Phase

**Phase 1: Sensor prototype**

The first goal is simply to connect the MPU-6050 to the Arduino and print stable motion/orientation readings to the Serial Monitor.

## Engineering Challenges

This project explores:

- I2C communication
- Accelerometers and gyroscopes
- Sensor calibration
- Signal filtering
- Threshold-based detection
- Transistor switching
- Feedback system design
- Human-centered testing
- Embedded firmware development

## Status

Early prototype / active development.
