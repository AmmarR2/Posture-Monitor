// Posture Monitor - Arduino prototype
//
// Initial milestone:
// 1. Connect the MPU-6050 over I2C.
// 2. Confirm sensor communication.
// 3. Print raw accelerometer/gyroscope values.
//
// The sensor-specific library and posture-detection logic will be added
// once the first hardware wiring test is complete.

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  Serial.println("Posture Monitor starting...");
  Serial.println("Milestone 1: MPU-6050 communication test");
}

void loop() {
  // Sensor-reading code will go here during the first prototype session.
  delay(1000);
}
