# Prototype Parts

## Core Electronics

- Arduino Uno R3
- GY-521 MPU-6050 module
- Breadboard
- Jumper wires
- LED
- 220–330 ohm resistor for LED
- Small DC vibration motor
- NPN transistor suitable for switching the vibration motor
- Flyback diode across the motor

## Useful Extras

- USB cable for Arduino programming
- Battery pack or portable power source for later testing
- Velcro, elastic strap, clip, or temporary mounting material
- Multimeter for debugging

## Notes

The first prototype does not need to be wearable. Start on the breadboard, prove that the posture detection works, then reduce the size later.

The vibration motor should not be powered directly from an Arduino I/O pin. Use the transistor as a switch and place a flyback diode across the motor to protect the circuit from voltage spikes.
