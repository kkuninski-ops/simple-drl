/*
 * Opel Vectra C 2008 — Gauge Needle Sweep
 * ATtiny85 implementation
 *
 * При всяко включване на KL15 стрелките се завъртат
 * до максимум и обратно преди таблото да поеме.
 *
 * Pinout:
 *   PB0 (Pin 5) → Stepper IN1 (100Ω в серия)
 *   PB1 (Pin 6) → Stepper IN2 (100Ω в серия)
 *   PB3 (Pin 2) → Stepper IN3 (100Ω в серия)
 *   PB4 (Pin 3) → Stepper IN4 (100Ω в серия)
 *   VCC (Pin 8) → +5V (78L05 от KL15)
 *   GND (Pin 4) → Маса
 *
 * Програмиране: Arduino as ISP @ 8MHz internal clock
 * Board: ATtiny85 (в Arduino IDE с ATtinyCore или ATtiny board package)
 */

#include <avr/sleep.h>
#include <avr/power.h>

// Пинове на стъпковия мотор
const uint8_t IN1 = PB0;
const uint8_t IN2 = PB1;
const uint8_t IN3 = PB3;
const uint8_t IN4 = PB4;

// ─── КАЛИБРАЦИОННИ СТОЙНОСТИ ──────────────────────────────────────────────
// Промени тези стойности ако стрелката не достига точно до максимума

// Брой half-steps от 0 до максимума на скоростомера (~220 km/h позиция)
// За VID29/X27.168 → 315 full-steps = 630 half-steps за 315°
// Скоростомер Vectra C: ~240° = ~480 half-steps (стартирай с 400, калибрирай)
const int MAX_STEPS     = 450;

// Брой стъпки назад за калибриране (трябва да е повече от максимума)
const int CALIB_STEPS   = 200;

// Скорост на движение нагоре (ms на стъпка) — по-малко = по-бързо
const uint8_t SPEED_UP  = 3;

// Скорост на движение надолу
const uint8_t SPEED_DOWN = 3;

// Пауза при максимума (ms)
const uint16_t HOLD_MS  = 350;

// Пауза след калибрирането
const uint16_t CALIB_HOLD_MS = 150;
// ──────────────────────────────────────────────────────────────────────────

// Half-step последователност (8 стъпки на цикъл — по-плавно движение)
const uint8_t HALF_STEP[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

int currentStep = 0;

void applyStep(int step) {
  int idx = ((step % 8) + 8) % 8;
  digitalWrite(IN1, HALF_STEP[idx][0]);
  digitalWrite(IN2, HALF_STEP[idx][1]);
  digitalWrite(IN3, HALF_STEP[idx][2]);
  digitalWrite(IN4, HALF_STEP[idx][3]);
}

void releaseCoils() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// Движи стрелката с зададен брой стъпки (+напред, -назад)
void moveSteps(int steps, uint8_t delayMs) {
  int dir   = (steps > 0) ? 1 : -1;
  int count = abs(steps);
  for (int i = 0; i < count; i++) {
    currentStep += dir;
    applyStep(currentStep);
    delay(delayMs);
  }
}

// Плавно ускоряване при старт на движението
void moveWithAccel(int totalSteps, uint8_t minDelay, uint8_t maxDelay) {
  int accelSteps = totalSteps / 4;
  int dir = (totalSteps > 0) ? 1 : -1;

  for (int i = 0; i < abs(totalSteps); i++) {
    uint8_t d;
    if (i < accelSteps) {
      // Ускорение
      d = maxDelay - ((maxDelay - minDelay) * i / accelSteps);
    } else if (i > abs(totalSteps) - accelSteps) {
      // Забавяне
      int j = abs(totalSteps) - i;
      d = maxDelay - ((maxDelay - minDelay) * j / accelSteps);
    } else {
      d = minDelay;
    }
    currentStep += dir;
    applyStep(currentStep);
    delay(d);
  }
}

void setup() {
  // Намали честотата на процесора за пестене на ток (по желание)
  // clock_prescale_set(clock_div_2); // 4MHz вместо 8MHz

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  releaseCoils();

  // Изчакай захранването да се стабилизира
  delay(400);

  // СТЪПКА 1: Калибрация — върни стрелката до хард-стоп (0 позиция)
  moveSteps(-CALIB_STEPS, 4);
  delay(CALIB_HOLD_MS);
  currentStep = 0;  // Нулиране на позиция

  // СТЪПКА 2: Плавно движение до максимума
  moveWithAccel(MAX_STEPS, SPEED_UP, 8);
  delay(HOLD_MS);

  // СТЪПКА 3: Плавно връщане до нулата
  moveWithAccel(-MAX_STEPS, SPEED_DOWN, 8);
  delay(100);

  // СТЪПКА 4: Освобождаване — таблото поема управлението
  releaseCoils();

  // ATtiny85 влиза в дълбок сън (power down) — не консумира ток
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
}

void loop() {
  // Никога не се достига — всичко се случва в setup()
}
