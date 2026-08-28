#include <Wire.h>
#include <EEPROM.h>
#include <math.h>

// ============================================================
// Posture Monitor - Arduino Uno + MPU-6050 + Vibration Motor
// ============================================================
// Hardware:
//   MPU-6050 VCC -> 5V
//   MPU-6050 GND -> GND
//   MPU-6050 SDA -> A4
//   MPU-6050 SCL -> A5
//
//   Arduino D9 -> 1k resistor -> S8050 base
//   S8050 emitter -> GND
//   S8050 collector -> vibration motor negative
//   vibration motor positive -> 5V
//
// Behavior:
//   - Calibrates the user's upright posture at startup.
//   - Detects posture angle using the MPU-6050 accelerometer.
//   - Requires bad posture to continue for 3 seconds before alerting.
//   - Vibrates while the user remains slouched after that delay.
//   - Tracks good/slouch time and number of slouch events.
//   - Saves statistics to EEPROM every 10 seconds.
//   - Prints live CSV-style statistics to Serial every second.
// ============================================================

const byte MPU_ADDR = 0x68;
const byte MOTOR_PIN = 9;

// Posture settings
const float SLOUCH_THRESHOLD_DEG = 12.0;      // Difference from calibrated posture
const unsigned long SLOUCH_DELAY_MS = 3000;   // Must slouch for 3 seconds
const unsigned long CALIBRATION_MS = 5000;    // Sit upright for first 5 seconds

// Timing
const unsigned long SAMPLE_INTERVAL_MS = 50;  // 20 Hz sensor sampling
const unsigned long SERIAL_INTERVAL_MS = 1000;
const unsigned long EEPROM_INTERVAL_MS = 10000;

// EEPROM layout / validation
const unsigned long EEPROM_MAGIC = 0x504F5354UL; // "POST"

struct SavedStats {
  unsigned long magic;
  unsigned long goodSeconds;
  unsigned long slouchSeconds;
  unsigned long slouchEvents;
};

SavedStats stats;

float baselinePitch = 0.0;
float filteredPitch = 0.0;
bool filterInitialized = false;

bool slouchCandidate = false;
bool slouchActive = false;
unsigned long slouchStartMs = 0;

unsigned long lastSampleMs = 0;
unsigned long lastSerialMs = 0;
unsigned long lastSaveMs = 0;
unsigned long lastStatsTickMs = 0;

// ------------------------------------------------------------
// MPU-6050 helpers
// ------------------------------------------------------------
void writeMPU(byte reg, byte value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

bool readMPURegister(byte startReg, byte *buffer, byte length) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(startReg);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  Wire.requestFrom(MPU_ADDR, length, true);

  if (Wire.available() < length) {
    return false;
  }

  for (byte i = 0; i < length; i++) {
    buffer[i] = Wire.read();
  }

  return true;
}

bool initializeMPU() {
  Wire.begin();
  Wire.setClock(400000);

  // Wake the MPU-6050 from sleep.
  writeMPU(0x6B, 0x00);
  delay(100);

  // Accelerometer +/-2g.
  writeMPU(0x1C, 0x00);

  // Gyroscope +/-250 deg/s.
  writeMPU(0x1B, 0x00);

  // Digital low-pass filter.
  writeMPU(0x1A, 0x03);

  byte whoAmI = 0;
  if (!readMPURegister(0x75, &whoAmI, 1)) {
    return false;
  }

  return (whoAmI == 0x68 || whoAmI == 0x69);
}

bool readPitch(float &pitchDeg) {
  byte data[6];

  if (!readMPURegister(0x3B, data, 6)) {
    return false;
  }

  int16_t axRaw = (int16_t)((data[0] << 8) | data[1]);
  int16_t ayRaw = (int16_t)((data[2] << 8) | data[3]);
  int16_t azRaw = (int16_t)((data[4] << 8) | data[5]);

  float ax = axRaw / 16384.0;
  float ay = ayRaw / 16384.0;
  float az = azRaw / 16384.0;

  // Pitch works well when the sensor tilts forward/backward with the torso.
  pitchDeg = atan2(ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
  return true;
}

// ------------------------------------------------------------
// EEPROM statistics
// ------------------------------------------------------------
void loadStats() {
  EEPROM.get(0, stats);

  if (stats.magic != EEPROM_MAGIC) {
    stats.magic = EEPROM_MAGIC;
    stats.goodSeconds = 0;
    stats.slouchSeconds = 0;
    stats.slouchEvents = 0;
    EEPROM.put(0, stats);
  }
}

void saveStats() {
  stats.magic = EEPROM_MAGIC;
  EEPROM.put(0, stats);
}

// ------------------------------------------------------------
// Calibration
// ------------------------------------------------------------
void calibratePosture() {
  Serial.println(F("CALIBRATION_START"));
  Serial.println(F("Sit upright and stay still for 5 seconds..."));

  unsigned long start = millis();
  float pitchSum = 0.0;
  unsigned int samples = 0;

  while (millis() - start < CALIBRATION_MS) {
    float pitch;

    if (readPitch(pitch)) {
      pitchSum += pitch;
      samples++;
    }

    delay(20);
  }

  if (samples > 0) {
    baselinePitch = pitchSum / samples;
    filteredPitch = baselinePitch;
    filterInitialized = true;
  }

  Serial.print(F("CALIBRATION_DONE,baseline="));
  Serial.println(baselinePitch, 2);
}

// ------------------------------------------------------------
// Posture logic
// ------------------------------------------------------------
void updatePosture(float pitch) {
  // Low-pass filter to prevent tiny movements/noise from causing alerts.
  if (!filterInitialized) {
    filteredPitch = pitch;
    filterInitialized = true;
  } else {
    const float alpha = 0.15;
    filteredPitch = (alpha * pitch) + ((1.0 - alpha) * filteredPitch);
  }

  float deviation = fabs(filteredPitch - baselinePitch);
  bool currentlyBad = deviation >= SLOUCH_THRESHOLD_DEG;
  unsigned long now = millis();

  if (currentlyBad) {
    if (!slouchCandidate) {
      slouchCandidate = true;
      slouchStartMs = now;
    }

    if (!slouchActive && (now - slouchStartMs >= SLOUCH_DELAY_MS)) {
      slouchActive = true;
      stats.slouchEvents++;
      Serial.println(F("SLOUCH_ALERT"));
    }
  } else {
    slouchCandidate = false;
    slouchActive = false;
  }

  digitalWrite(MOTOR_PIN, slouchActive ? HIGH : LOW);
}

void updateTimeStats() {
  unsigned long now = millis();

  if (now - lastStatsTickMs >= 1000) {
    unsigned long elapsedSeconds = (now - lastStatsTickMs) / 1000;
    lastStatsTickMs += elapsedSeconds * 1000;

    // Count the full candidate period as slouch time, even before vibration.
    if (slouchCandidate) {
      stats.slouchSeconds += elapsedSeconds;
    } else {
      stats.goodSeconds += elapsedSeconds;
    }
  }
}

void printStatistics() {
  unsigned long totalSeconds = stats.goodSeconds + stats.slouchSeconds;
  float goodPercent = 100.0;

  if (totalSeconds > 0) {
    goodPercent = (100.0 * stats.goodSeconds) / totalSeconds;
  }

  float deviation = fabs(filteredPitch - baselinePitch);

  Serial.print(F("DATA,"));
  Serial.print(F("pitch="));
  Serial.print(filteredPitch, 2);
  Serial.print(F(",baseline="));
  Serial.print(baselinePitch, 2);
  Serial.print(F(",deviation="));
  Serial.print(deviation, 2);
  Serial.print(F(",posture="));
  Serial.print(slouchCandidate ? F("SLOUCH") : F("GOOD"));
  Serial.print(F(",alert="));
  Serial.print(slouchActive ? 1 : 0);
  Serial.print(F(",good_s="));
  Serial.print(stats.goodSeconds);
  Serial.print(F(",slouch_s="));
  Serial.print(stats.slouchSeconds);
  Serial.print(F(",events="));
  Serial.print(stats.slouchEvents);
  Serial.print(F(",good_pct="));
  Serial.println(goodPercent, 1);
}

// ------------------------------------------------------------
// Arduino setup / loop
// ------------------------------------------------------------
void setup() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  Serial.begin(115200);
  delay(500);

  Serial.println(F("POSTURE_MONITOR_START"));

  loadStats();

  if (!initializeMPU()) {
    Serial.println(F("ERROR: MPU-6050 not detected. Check SDA/SCL/VCC/GND wiring."));

    while (true) {
      // Fast pulses indicate a sensor/wiring error.
      digitalWrite(MOTOR_PIN, HIGH);
      delay(100);
      digitalWrite(MOTOR_PIN, LOW);
      delay(900);
    }
  }

  Serial.println(F("MPU6050_OK"));
  calibratePosture();

  unsigned long now = millis();
  lastSampleMs = now;
  lastSerialMs = now;
  lastSaveMs = now;
  lastStatsTickMs = now;

  Serial.println(F("SYSTEM_READY"));
}

void loop() {
  unsigned long now = millis();

  if (now - lastSampleMs >= SAMPLE_INTERVAL_MS) {
    lastSampleMs = now;

    float pitch;
    if (readPitch(pitch)) {
      updatePosture(pitch);
    }
  }

  updateTimeStats();

  if (now - lastSerialMs >= SERIAL_INTERVAL_MS) {
    lastSerialMs = now;
    printStatistics();
  }

  if (now - lastSaveMs >= EEPROM_INTERVAL_MS) {
    lastSaveMs = now;
    saveStats();
  }
}
