#include <TM1637Display.h>

// ---------- Пины ----------
const byte SW_PIN_1   = 3;    // 2-позиционный переключатель: контакт 1
const byte SW_PIN_2   = 4;    // 2-позиционный переключатель: контакт 2
const byte BTN_PIN    = 5;    // Кнопка
const byte BUZZ_PIN   = 6;    // Пищалка (tone)
const byte LED_PIN    = 13;   // Встроенный LED
const byte POT_PIN    = A2;   // Потенциометр

// TM1637
const byte TM_CLK     = 8;    // CLK дисплея
const byte TM_DIO     = 9;    // DIO дисплея

// ---------- Настройки ----------
unsigned int BUZZF = 2000;    // Частота пищалки, Гц
byte DISP_BRIGHT   = 7;       // Яркость 0..7

TM1637Display display(TM_CLK, TM_DIO);

// Для отслеживания нажатия кнопки
bool lastBtn = HIGH;

// ---------- Вспомогательные функции ----------
void blinkLedTwice() {
  for (byte i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(120);
    digitalWrite(LED_PIN, HIGH);
    delay(120);
  }
}

void showNumber(int value) {
  // Показываем число 0..9999 (у нас 0..250)
  display.showNumberDec(value, false);
}

void rampToNumber(int target) {
  // Быстрое "нарастание числа" от 0 до target
  target = constrain(target, 0, 250);

  int stepDelay = 6; // мс между шагами (быстро)
  int stepSize  = 1; // шаг по умолчанию

  // Для больших чисел ускоряем, чтобы анимация оставалась быстрой
  if (target > 120) stepSize = 2;
  if (target > 200) stepSize = 3;

  for (int v = 0; v <= target; v += stepSize) {
    showNumber(v);
    delay(stepDelay);
  }

  // Гарантированно показать точное целевое
  showNumber(target);
}

void runCycle(int P) {
  P = constrain(P, 0, 250);

  // 1) Анимация нарастания до значения
  rampToNumber(P);

  // 2) Пищалка на длительность P*3 мс
  unsigned long buzzTime = (unsigned long)P * 3UL;
  if (buzzTime > 0) {
    tone(BUZZ_PIN, BUZZF);
    delay(buzzTime);
    noTone(BUZZ_PIN);
  } else {
    noTone(BUZZ_PIN);
  }

  // 3) После цикла число остаётся на экране, LED мигает 2 раза
  blinkLedTwice();
}

void setup() {
  pinMode(SW_PIN_1, INPUT_PULLUP);
  pinMode(SW_PIN_2, INPUT_PULLUP);
  pinMode(BTN_PIN,  INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUZZ_PIN, OUTPUT);
  noTone(BUZZ_PIN);

  display.setBrightness(DISP_BRIGHT, true);
  display.clear();
  showNumber(0);

  // Немного шума для random()
  randomSeed(analogRead(A0));
}

void loop() {
  // Режимы через SPDT: общий контакт на GND, выходы на D3/D4
  bool mode1 = (digitalRead(SW_PIN_1) == LOW); // режим случайного P
  bool mode2 = (digitalRead(SW_PIN_2) == LOW); // режим от потенциометра

  // LED включён, если режим определён
  digitalWrite(LED_PIN, (mode1 || mode2) ? HIGH : LOW);

  // Кнопка нажата = LOW (INPUT_PULLUP)
  bool btnNow = digitalRead(BTN_PIN);

  // Срабатывание по фронту нажатия
  if (lastBtn == HIGH && btnNow == LOW) {
    if (mode1 && !mode2) {
      int P = random(0, 251);      // 0..250
      runCycle(P);
    } else if (mode2 && !mode1) {
      int raw = analogRead(POT_PIN);          // 0..1023
      int Ppot = map(raw, 0, 1023, 0, 250);   // 0..250
      runCycle(Ppot);
    }
    // Если оба/ни один режим не активны — ничего не делаем
  }

  lastBtn = btnNow;
  delay(15); // простой антидребезг
}