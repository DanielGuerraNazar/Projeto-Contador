#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Setup do pinout do arduino e outros..
void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  randomSeed(analogRead(0));
  lcd.print("Vamos Jogar!!");
  delay(5000);
}

void loop() {
  // Randomiza um numero entre 1 e 10 e mostra na tela a quantidade de bolinhas
  // correspondente
  int quantidadeDeLeds = 9;
  bool jogoAtivo = true;
  int numero = random(1, 10);
  lcd.clear();
  lcd.print("Memorize:");
  lcd.setCursor(0, 1);
  for (int i = 0; i < numero; i++) {
    lcd.print("* ");
  }

  delay(3000);

  lcd.clear();
  lcd.print("Quantas bolinhas?");

  int contadorDeApertos = 0;
  while (jogoAtivo) {
    if (digitalRead(12) == LOW) {
      contadorDeApertos++;
      if (contadorDeApertos <= quantidadeDeLeds) {
        digitalWrite(contadorDeApertos + 1, HIGH);
      } else {

        contadorDeApertos = quantidadeDeLeds;
      }
      delay(200);
    }
    if (digitalRead(11) == 0)
      jogoAtivo = false;
  }
  lcd.clear();

  if (contadorDeApertos == numero) {
    lcd.print("ACERTOU!");
    tone(13, 1319, 120); // E6
    delay(150);
    tone(13, 1568, 120); // G6
    delay(150);
    tone(13, 1976, 180); // B6
    delay(200);
    noTone(13);
    delay(1000);
  } else {
    lcd.print("ERROU!");
    lcd.setCursor(0, 1);
    lcd.print("Era: ");
    lcd.print(numero);
    tone(13, 500, 300);
    delay(350);
    tone(13, 300, 400);
    delay(2000);
  }

  if (!jogoAtivo) {
    for (int i = 2; i < quantidadeDeLeds + 2; i++) {
      digitalWrite(i, LOW);
    }
    lcd.clear();
    jogoAtivo = true;
    contadorDeApertos = 0;
  }
  delay(2000);
}