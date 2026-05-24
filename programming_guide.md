# Ръководство за програмиране на ATtiny85
## Използва се за Circuit 3 (Dial Sweep) и Circuit 4 (S32 → Radio Vol)

---

## Какво ти трябва

| Хардуер | Описание |
|---------|---------|
| Arduino Uno или Nano | Служи като ISP програматор |
| ATtiny85 (DIP-8) | Чипчето, което се програмира |
| Jumper кабели | 6 броя |
| Breadboard | За временно свързване |
| USB кабел | За Arduino |

**Софтуер:**
- Arduino IDE (безплатен, от arduino.cc)
- ATtiny85 Board Package

---

## Стъпка 1 — Инсталиране на ATtiny85 поддръжка в Arduino IDE

### Arduino IDE 2.x

1. Отвори Arduino IDE
2. **File → Preferences**
3. В полето **"Additional boards manager URLs"** добави:
   ```
   https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json
   ```
4. **Tools → Board → Boards Manager**
5. Търси `attiny` → Инсталирай **"ATtiny" by David A. Mellis**
6. Затвори Boards Manager

---

## Стъпка 2 — Качи "Arduino as ISP" скеч на Arduino Uno/Nano

1. Свържи Arduino Uno/Nano с USB
2. **File → Examples → 11.ArduinoISP → ArduinoISP**
3. **Tools → Board → Arduino Uno** (или Nano)
4. **Tools → Port** → избери правилния COM порт
5. **Sketch → Upload** — качи скеча

Arduino вече е ISP програматор.

---

## Стъпка 3 — Свързване на ATtiny85 към Arduino

```
Arduino Uno                    ATtiny85 (DIP-8)
                               ┌───────────┐
5V  ─────────────────────────► │ 8  VCC    │
GND ─────────────────────────► │ 4  GND    │
Pin 13 (SCK)  ──────────────► │ 7  PB2    │
Pin 12 (MISO) ──────────────► │ 6  PB1    │
Pin 11 (MOSI) ──────────────► │ 5  PB0    │
Pin 10 (SS/RESET) ──────────► │ 1  PB5/RST│
                               └───────────┘

ВАЖНО: Постави 10µF кондензатор между RESET и GND на Arduino Uno
       (+ страна към RESET, - страна към GND)
       БЕЗ него Arduino Uno се рестартира по време на програмиране!
```

**Схема:**
```
Arduino                         ATtiny85
  13 SCK ────────────────────── Pin 7 (PB2)
  12 MISO ───────────────────── Pin 6 (PB1)
  11 MOSI ───────────────────── Pin 5 (PB0)
  10 SS  ────────────────────── Pin 1 (RESET)
   5V    ────────────────────── Pin 8 (VCC)
  GND    ────────────────────── Pin 4 (GND)

Arduino RESET──[10µF]──GND  ← Задължително за Uno!
```

---

## Стъпка 4 — Конфигурация в Arduino IDE за ATtiny85

1. **Tools → Board → ATtiny Microcontrollers → ATtiny25/45/85**
2. **Tools → Processor: ATtiny85**
3. **Tools → Clock: Internal 8 MHz**  ← **Важно! НЕ избирай 1MHz или External**
4. **Tools → Programmer: Arduino as ISP**
5. **Tools → Port** → COM порта на Arduino

---

## Стъпка 5 — Изгаряне на Bootloader (задължително веднъж за нов чип)

**Tools → Burn Bootloader**

Това задава fuse битовете — настройва вътрешния часовник на 8MHz.  
Трябва да се направи само **веднъж** за всеки нов ATtiny85.

Очаквано съобщение: `Done burning bootloader`

Ако получиш грешка:
- Провери кондензатора 10µF на Arduino RESET
- Провери свързванията
- Провери дали "ArduinoISP" скечът е качен успешно

---

## Стъпка 6 — Качване на скеча

1. Отвори `.ino` файла (напр. `attiny85_sweep.ino`)
2. **Sketch → Upload Using Programmer**  
   (**НЕ** обикновения Upload бутон — той не работи за ATtiny!)
3. Изчакай: `Done uploading`

---

## Проверка след програмиране

Извади ATtiny85 от програматора и го постави в схемата.  
При подаване на напрежение трябва да виждаш LED или стрелката да се движи.

---

## Честа грешка: Синхронизация

```
avrdude: stk500_recv(): programmer is not responding
```

**Решения:**
1. Провери кондензатора 10µF (за Uno)
2. Провери всички 6 кабела
3. Провери Port и Programmer в Tools
4. Опитай без кондензатор ако ползваш Arduino Nano

---

## Честа грешка: Wrong signature

```
avrdude: Expected signature for ATtiny85
```

**Решение:** Провери Tools → Processor → ATtiny85 (не ATtiny45!)

---

## Резюме на настройките

| Настройка | Стойност |
|-----------|---------|
| Board | ATtiny25/45/85 |
| Processor | ATtiny85 |
| Clock | Internal 8 MHz |
| Programmer | Arduino as ISP |
| Upload метод | Sketch → Upload Using Programmer |
