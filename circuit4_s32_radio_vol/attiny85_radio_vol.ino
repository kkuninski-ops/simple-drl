/*
 * Opel Vectra C 2008 — S32 Dimmer Knob → Radio Volume Control
 * ATtiny85 implementation
 *
 * Чете напрежението на S32 реостата и симулира SWRC бутони
 * (VOL+/VOL-) към Opel радиото при завъртане на врътката.
 *
 * S32 продължава да управлява ILL+ нормално — не е прекъсната тази функция!
 *
 * Pinout:
 *   PB2 (Pin 7, ADC1)  → S32 wiper (напрежение 0–5V след делител)
 *   PB0 (Pin 5)        → VOL+ (BC547 база, 10kΩ в серия)
 *   PB1 (Pin 6)        → VOL- (BC547 база, 10kΩ в серия)
 *   VCC (Pin 8)        → +5V (78L05 от KL15)
 *   GND (Pin 4)        → Маса
 *
 * Програмиране: Arduino as ISP @ 8MHz internal clock
 */

#include <avr/io.h>
#include <util/delay.h>

// ─── КОНФИГУРАЦИЯ ─────────────────────────────────────────────────────────

// Пинове
#define PIN_ADC       1       // ADC channel 1 = PB2 (физически Pin 7)
#define PIN_VOL_UP    PB0     // Pin 5 → VOL+ транзистор
#define PIN_VOL_DOWN  PB1     // Pin 6 → VOL- транзистор

// Прагов брой ADC единици за засичане на завъртане
// ADC е 10-bit (0-1023). При 10kΩ делител, S32 дава ~512 при среда.
// Ако S32 е завъртян с ~5% → промяна от ~50 единици.
#define THRESHOLD     35      // Минимална промяна за регистриране (0-1023)

// Колко дълго да е "натиснат" симулираният бутон (ms)
#define PULSE_MS      80

// Колко дълго да чака между отчитания (ms)
// По-малко = по-чувствително завъртане
#define SAMPLE_MS     100

// Максимален брой VOL стъпки на едно завъртане
// (предотвратява прескачане при бързо завъртане)
#define MAX_STEPS     5

// ──────────────────────────────────────────────────────────────────────────

// ADC четене (10-bit, 0-1023)
uint16_t readADC() {
  ADMUX  = (1 << ADLAR) | PIN_ADC;         // Left-adjust, channel
  ADMUX |= (0 << REFS1) | (0 << REFS0);    // VCC as reference
  ADCSRA = (1 << ADEN) | (1 << ADSC)       // Enable + Start
         | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler /128

  while (ADCSRA & (1 << ADSC));            // Изчакай края на конверсията
  return ADC;
}

void pulseVolumeUp(uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    PORTB |=  (1 << PIN_VOL_UP);           // Транзисторът включен → VOL+
    _delay_ms(PULSE_MS);
    PORTB &= ~(1 << PIN_VOL_UP);           // Транзисторът изключен
    if (i < times - 1) _delay_ms(40);      // Пауза между повторни натисквания
  }
}

void pulseVolumeDown(uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    PORTB |=  (1 << PIN_VOL_DOWN);         // Транзисторът включен → VOL-
    _delay_ms(PULSE_MS);
    PORTB &= ~(1 << PIN_VOL_DOWN);         // Транзисторът изключен
    if (i < times - 1) _delay_ms(40);
  }
}

int main(void) {
  // Настройка на пинове
  DDRB |=  (1 << PIN_VOL_UP);             // PB0 = изход
  DDRB |=  (1 << PIN_VOL_DOWN);           // PB1 = изход
  DDRB &= ~(1 << PB2);                    // PB2 = вход (ADC)

  PORTB &= ~(1 << PIN_VOL_UP);            // Изходите започват LOW
  PORTB &= ~(1 << PIN_VOL_DOWN);

  // Изчакай захранването да се стабилизира
  _delay_ms(500);

  // Начална стойност — прочети S32 позицията при старт
  uint16_t prevValue = readADC();
  _delay_ms(SAMPLE_MS);

  while (1) {
    uint16_t currValue = readADC();

    // Изчисли разликата (с посока)
    int16_t delta = (int16_t)currValue - (int16_t)prevValue;

    if (delta > THRESHOLD) {
      // Завъртане по посока на часовника → VOL+
      uint8_t steps = (uint8_t)(delta / THRESHOLD);
      if (steps > MAX_STEPS) steps = MAX_STEPS;
      pulseVolumeUp(steps);
      prevValue = currValue;

    } else if (delta < -THRESHOLD) {
      // Завъртане обратно на часовника → VOL-
      uint8_t steps = (uint8_t)((-delta) / THRESHOLD);
      if (steps > MAX_STEPS) steps = MAX_STEPS;
      pulseVolumeDown(steps);
      prevValue = currValue;
    }

    _delay_ms(SAMPLE_MS);
  }

  return 0;
}
