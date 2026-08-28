# Posture Monitor Project Plan

## Phase 1 — Sensor Bring-Up

- Wire the MPU-6050 to the Arduino over I2C.
- Confirm the sensor is detected.
- Print raw accelerometer and gyroscope values.

## Phase 2 — Orientation

- Convert raw data into useful tilt/orientation measurements.
- Compare accelerometer-only and filtered estimates.
- Choose stable values for posture detection.

## Phase 3 — Calibration

- Record the user's normal upright posture.
- Store a baseline angle.
- Add a calibration button or startup routine later if useful.

## Phase 4 — Slouch Detection

- Define an angle difference that counts as poor posture.
- Require the threshold to be exceeded for a short period before triggering an alert.
- Add hysteresis/deadband to reduce rapid on/off behavior.

## Phase 5 — Feedback

- Start with an LED warning.
- Add the vibration motor through an NPN transistor.
- Tune alert duration and cooldown so reminders are noticeable but not annoying.

## Phase 6 — Testing

Test several situations:

- Upright sitting
- Slight lean
- Clear slouch
- Reaching forward briefly
- Standing up
- Moving around at the desk

Track false positives and adjust the algorithm.

## Phase 7 — Miniaturization

After the breadboard prototype works reliably:

- Replace the large Arduino prototype with a smaller microcontroller if desired.
- Choose a battery.
- Mount the sensor securely.
- Design a clip, strap, or enclosure.

## Main Engineering Question

Can a low-cost inertial sensor distinguish sustained slouching from normal short-term body movement well enough to provide useful posture reminders?
