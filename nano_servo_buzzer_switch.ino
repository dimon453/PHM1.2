#include <Servo.h>

// ---------- Пины ----------
const byte SW_PIN_1   = 3;    // 2-позиционный переключатель: контакт 1
const byte SW_PIN_2   = 4;    // 2-позиционный переключатель: контакт 2
const byte BTN_PIN    = 5;    // Кнопка
const byte BUZZ_PIN   = 6;    // Пищалка (tone)
const byte SERVO_PIN  = 9;    // Сигнал сервопривода
const byte LED_PIN    = 13;   // Встроенный LED (можно вынести на отдельный пин)
const byte POT_PIN    = A2;   // Потенциометр

// ---------- Настройки ----------
int Amin  = 30;               // Мин. угол серво
int Amax  = 90;               // Макс. угол серво
unsigned int BUZZF = 2000;    // Частота пищалки, Гц

Servo myServo;

// Для отслеживания нажатия (антидребезг простым фронтом)
bool lastBtn = HIGH;

// ---------- Вспомогательные функции ----------
int mapPToAngle(int pVal) {
  // Пропорция: 0..250 -> Amin..Amax
  return map(pVal, 0, 250, Amin, Amax);
}

void blinkLedTwice() {
  for (byte i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, LOW);   // гасим
    delay(150);
    digitalWrite(LED_PIN, HIGH);  // зажигаем
    delay(150);
  }
}

// Выполнить цикл: серво + пищалка + 2 мигания
void runCycle(int P) {
  P = constrain(P, 0, 250);

  int angle = mapPToAngle(P);
  myServo.write(angle);           // Серво в целевой угол (и остаётся там)

  unsigned long buzzTime = (unsigned long)P * 3UL; // длительность, мс
  if (buzzTime > 0) {
    tone(BUZZ_PIN, BUZZF);        // Старт писка
    delay(buzzTime);              // Ждём P*3 мс
    noTone(BUZZ_PIN);             // Стоп писка
  } else {
    noTone(BUZZ_PIN);
  }

  blinkLedTwice();                // Индикация завершения
}

void setup() {
  pinMode(SW_PIN_1, INPUT_PULLUP);
  pinMode(SW_PIN_2, INPUT_PULLUP);
  pinMode(BTN_PIN,  INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUZZ_PIN, OUTPUT);
  noTone(BUZZ_PIN);

  myServo.attach(SERVO_PIN);
  myServo.write(Amin);            // Стартовое положение
}

void loop() {
  // Определяем положение 2-позиционного переключателя:
  // Вариант подключения SPDT: общий контакт на GND,
  // два выхода на SW_PIN_1 и SW_PIN_2 (с INPUT_PULLUP).
  bool mode1 = (digitalRead(SW_PIN_1) == LOW); // положение 1 активно
  bool mode2 = (digitalRead(SW_PIN_2) == LOW); // положение 2 активно

  // LED включён, если любой режим активен
  digitalWrite(LED_PIN, (mode1 || mode2) ? HIGH : LOW);

  // Кнопка: с INPUT_PULLUP нажата = LOW
  bool btnNow = digitalRead(BTN_PIN);

  // Обработка только по фронту нажатия
  if (lastBtn == HIGH && btnNow == LOW) {
    if (mode1 && !mode2) {
      // Режим 1: случайный P (0..250)
      int P = random(0, 251);
      runCycle(P);

    } else if (mode2 && !mode1) {
      // Режим 2: Ppot с потенциометра (0..250)
      int raw = analogRead(POT_PIN);          // 0..1023
      int Ppot = map(raw, 0, 1023, 0, 250);
      runCycle(Ppot);
    }
    // Если переключатель в некорректном состоянии (оба/ни один) — ничего не делаем
  }

  lastBtn = btnNow;
  delay(15); // небольшой антидребезг
}