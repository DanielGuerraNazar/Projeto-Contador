#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <iostream>

LiquidCrystal_I2C lcd(32, 16, 2);
//Setup do pinout do arduino e outros..
void setup()
{
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
	// Saída para debug
	// Serial.begin(9600);
	delay(5000);
}

void loop()
{
	// Randomiza um numero entre 1 e 10 e mostra na tela a quantidade de bolinhas correspondente
	int quantidadeDeLeds = 9;
	bool jogoAtivo = true;
	int numero = random(1, 10); // Limite final da função random é excluído
	lcd.clear();
	lcd.print("Memorize:");
	lcd.setCursor(0, 1);
	for (int i = 0; i < numero; i++) {
		lcd.print("* ");
	}

	delay(10000);

	lcd.clear();
	lcd.print("Quantas bolinhas?");

	// Contador para o numero de bolinhas que o jogador acha que tem, e um loop para ler os botoes e acender os leds de acordo com este número
	int contadorDeApertos = 0;
	while (jogoAtivo) {
		if (digitalRead(12) == LOW) {
			contadorDeApertos++;
			if (contadorDeApertos <= quantidadeDeLeds) {
				digitalWrite(contadorDeApertos + 1, HIGH);
			}
			else {
				// Como não é possível diminuir o numero de bolinhas indicado pelo led, o contador é limitado a quantidade de leds
				contadorDeApertos = quantidadeDeLeds;
			}
			delay(200);
			// Saída para debug
			// Serial.print(contadorDeApertos);
		}
		if (digitalRead(11) == LOW) jogoAtivo = false;
	}

	lcd.clear();

	// Condicional final para verificar se o jogador acertou ou errou, e tocar uma musica de acordo com o resultado
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
	}
	else {
		lcd.print("ERROU!");
		lcd.setCursor(0, 1);
		lcd.print("Era: ");
		lcd.print(numero);
		tone(13, 500, 300);
		delay(350);
		tone(13, 300, 400);
        delay(2000);
	}

	// Reset dos leds e da tela para a proxima rodada
	if (!jogoAtivo) {
		for (int i = 2; i < quantidadeDeLeds + 2; i++)
		{
			digitalWrite(i, LOW);
		}
		lcd.clear();
		// Reset redundante para garantir que o jogo volte ao estado inicial
		jogoAtivo = true;
		contadorDeApertos = 0;
	}

	delay(1000);
}