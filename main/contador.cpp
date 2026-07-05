#include <Arduino.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>

// Endereço I2C do LCD (32 é 0x20 decimal, mantido do projeto original do
// usuário)
LiquidCrystal_I2C lcd(32, 16, 2);

// Definição de pinos segura para o ESP32
#define PIN_LED_1 13
#define PIN_LED_2 12
#define PIN_LED_3 14
#define PIN_LED_4 27
#define PIN_LED_5 26
#define PIN_LED_6 25
#define PIN_LED_7 33
#define PIN_LED_8 32
#define PIN_LED_9 4

const int pinsLeds[9] = {PIN_LED_1, PIN_LED_2, PIN_LED_3, PIN_LED_4, PIN_LED_5,
                         PIN_LED_6, PIN_LED_7, PIN_LED_8, PIN_LED_9};

#define PIN_BTN_INC 23
#define PIN_BTN_SUBMIT 19
#define PIN_BUZZER 18

// Variáveis Globais de Gamificação
int level = 1;
int xp = 0;
int score = 0;
int comboStreak = 0;

// Estatísticas salvas na memória Flash (Preferences)
int highScore = 0;
int totalPartidas = 0;
int totalAcertos = 0;
int nivelMaximo = 1;
float tempoSomaReacao = 0.0;

String statusJogo = "Inicializando...";

// Instâncias das bibliotecas
Preferences preferences;
WebServer server(80);

// Estrutura para armazenamento do histórico local na RAM (últimas 10 partidas)
struct RegistroPartida {
  int partida;
  int nivel;
  int numeroAlvo;
  int numeroPalpite;
  float tempoReacao;
  bool acertou;
  bool timeout;
};

const int MAX_HISTORICO = 10;
RegistroPartida historicoPartidas[MAX_HISTORICO];
int historicoCount = 0;

#include "site.h"

// Delay não-bloqueante para manter o Web Server responsivo
void nonBlockingDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    server.handleClient();
    delay(1);
  }
}

// Adiciona uma partida ao histórico local
void adicionarHistorico(int nivel, int target, int guess, float time, bool hit,
                        bool timeout) {
  for (int i = MAX_HISTORICO - 1; i > 0; i--) {
    historicoPartidas[i] = historicoPartidas[i - 1];
  }

  historicoPartidas[0].partida = totalPartidas;
  historicoPartidas[0].nivel = nivel;
  historicoPartidas[0].numeroAlvo = target;
  historicoPartidas[0].numeroPalpite = guess;
  historicoPartidas[0].tempoReacao = time;
  historicoPartidas[0].acertou = hit;
  historicoPartidas[0].timeout = timeout;

  if (historicoCount < MAX_HISTORICO) {
    historicoCount++;
  }
}

void setup() {
  // Inicializa barramento I2C com os pinos padrão do ESP32 (SDA=21, SCL=22)
  Wire.begin(21, 22);

  // Setup do pinout do ESP32 para LEDs
  for (int i = 0; i < 9; i++) {
    pinMode(pinsLeds[i], OUTPUT);
    digitalWrite(pinsLeds[i], LOW);
  }

  pinMode(PIN_BTN_INC, INPUT_PULLUP);
  pinMode(PIN_BTN_SUBMIT, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);

  // LCD Init
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Vamos Jogar!!");
  lcd.setCursor(0, 1);
  lcd.print("Conectando WiFi...");

  Serial.begin(115200);

  // Carrega estatísticas do armazenamento flash persistente
  preferences.begin("game_stats", false);
  highScore = preferences.getInt("highScore", 0);
  totalPartidas = preferences.getInt("totalPartidas", 0);
  totalAcertos = preferences.getInt("totalAcertos", 0);
  nivelMaximo = preferences.getInt("nivelMaximo", 1);
  tempoSomaReacao = preferences.getFloat("tempoSomaReacao", 0.0);

  level = nivelMaximo; // inicia no maior nível já alcançado

  // Inicialização da conexão WiFi
  WiFi.begin("Jogo-Contador-WiFi", "12345678");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.print("WiFi Conectado!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
  } else {
    // Fallback: se falhar, cria sua própria rede (Access Point)
    Serial.println("\nFalha de conexao. Iniciando AP...");
    WiFi.softAP("Jogo-Contador-AP");
    Serial.print("IP do AP: ");
    Serial.println(WiFi.softAPIP());
    lcd.clear();
    lcd.print("AP: Jogo-Contador");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.softAPIP().toString());
  }

  // Rotas do Web Server
  server.on("/", HTTP_GET, []() { server.send(200, "text/html", index_html); });

  server.on("/api/stats", HTTP_GET, []() {
    String json = "{";
    json += "\"level\":" + String(level) + ",";
    json += "\"xp\":" + String(xp) + ",";
    json += "\"score\":" + String(score) + ",";
    json += "\"combo\":" + String(comboStreak) + ",";
    json += "\"highScore\":" + String(highScore) + ",";
    json += "\"totalPartidas\":" + String(totalPartidas) + ",";
    json += "\"totalAcertos\":" + String(totalAcertos) + ",";

    float taxa = 0.0;
    if (totalPartidas > 0) {
      taxa = ((float)totalAcertos / totalPartidas) * 100.0;
    }
    json += "\"taxaAcertos\":" + String(taxa, 1) + ",";

    float tMedio = 0.0;
    if (totalAcertos > 0) {
      tMedio = tempoSomaReacao / totalAcertos;
    }
    json += "\"tempoMedioReacao\":" + String(tMedio, 2) + ",";
    json += "\"status\":\"" + statusJogo + "\",";

    json += "\"historico\":[";
    for (int i = 0; i < historicoCount; i++) {
      json += "{";
      json += "\"partida\":" + String(historicoPartidas[i].partida) + ",";
      json += "\"nivel\":" + String(historicoPartidas[i].nivel) + ",";
      json += "\"numeroAlvo\":" + String(historicoPartidas[i].numeroAlvo) + ",";
      json +=
          "\"numeroPalpite\":" + String(historicoPartidas[i].numeroPalpite) +
          ",";
      json += "\"tempoReacao\":" + String(historicoPartidas[i].tempoReacao, 2) +
              ",";
      json += "\"acertou\":" +
              String(historicoPartidas[i].acertou ? "true" : "false") + ",";
      json += "\"timeout\":" +
              String(historicoPartidas[i].timeout ? "true" : "false");
      json += "}";
      if (i < historicoCount - 1)
        json += ",";
    }
    json += "]";
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/api/config", HTTP_POST, []() {
    if (server.hasArg("level")) {
      int newLvl = server.arg("level").toInt();
      if (newLvl >= 1 && newLvl <= 5) {
        level = newLvl;
        xp = 0;
      }
    }
    if (server.hasArg("reset")) {
      preferences.clear();
      highScore = 0;
      totalPartidas = 0;
      totalAcertos = 0;
      nivelMaximo = 1;
      tempoSomaReacao = 0.0;
      level = 1;
      xp = 0;
      score = 0;
      comboStreak = 0;
      historicoCount = 0;
    }
    server.send(200, "text/plain", "Configuracao Atualizada");
  });

  // Handler de Upload do OTA via Web
  server.on(
      "/update", HTTP_POST,
      []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html",
                    Update.hasError() ? "Falha na atualizacao!"
                                      : "<h3>Atualizacao efetuada com sucesso! "
                                        "Reiniciando o ESP32...</h3>");
        delay(1000);
        ESP.restart();
      },
      []() {
        HTTPUpload &upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
          Serial.printf("Update: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
          }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
          if (Update.write(upload.buf, upload.currentSize) !=
              upload.currentSize) {
            Update.printError(Serial);
          }
        } else if (upload.status == UPLOAD_FILE_END) {
          if (Update.end(true)) {
            Serial.printf("Update completo: %u bytes\n", upload.totalSize);
          } else {
            Update.printError(Serial);
          }
        }
      });

  server.begin();
  nonBlockingDelay(3000);

  randomSeed(analogRead(0));
}

void loop() {
  // 1. Definição da dificuldade com base no nível
  int maxNumero = 5;
  unsigned long tempoMemorizar = 8000;
  unsigned long tempoLimite = 15000;

  switch (level) {
  case 1:
    maxNumero = 5;
    tempoMemorizar = 8000;
    tempoLimite = 15000;
    break;
  case 2:
    maxNumero = 7;
    tempoMemorizar = 6000;
    tempoLimite = 12000;
    break;
  case 3:
    maxNumero = 9;
    tempoMemorizar = 5000;
    tempoLimite = 10000;
    break;
  case 4:
    maxNumero = 9;
    tempoMemorizar = 3000;
    tempoLimite = 8000;
    break;
  case 5:
    maxNumero = 9;
    tempoMemorizar = 2000;
    tempoLimite = 6000;
    break;
  }

  statusJogo = "Novo Round...";
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nivel ");
  lcd.print(level);
  lcd.setCursor(0, 1);
  lcd.print("XP: ");
  lcd.print(xp);
  lcd.print("/30  Sc:");
  lcd.print(score);

  nonBlockingDelay(2500);

  // 2. Randomização do número de asteriscos
  int numero = random(1, maxNumero + 1);
  statusJogo = "Memorizando...";
  lcd.clear();
  lcd.print("Memorize:");
  lcd.setCursor(0, 1);
  for (int i = 0; i < numero; i++) {
    lcd.print("* ");
  }

  nonBlockingDelay(tempoMemorizar);

  // 3. Fase de inserção da resposta com tempo limite
  lcd.clear();
  lcd.print("Quantas bolinhas?");

  statusJogo = "Aguardando Jogador";
  int contadorDeApertos = 0;
  bool respondeu = false;
  bool timedOut = false;
  unsigned long tempoInicioResposta = millis();

  // Desliga todos os leds para a nova resposta
  for (int i = 0; i < 9; i++) {
    digitalWrite(pinsLeds[i], LOW);
  }

  while (!respondeu && !timedOut) {
    server.handleClient(); // Mantém web server rodando

    unsigned long decorrido = millis() - tempoInicioResposta;
    int tempoRestante = (tempoLimite - decorrido) / 1000;

    if (decorrido >= tempoLimite) {
      timedOut = true;
      break;
    }

    // Atualiza o display LCD com o palpite atual e cronômetro
    lcd.setCursor(0, 1);
    lcd.print("Cliques: ");
    lcd.print(contadorDeApertos);
    lcd.print(" T: ");
    lcd.print(tempoRestante);
    lcd.print("s  ");

    // Detecção de botão de incremento
    if (digitalRead(PIN_BTN_INC) == LOW) {
      contadorDeApertos++;
      if (contadorDeApertos <= 9) {
        digitalWrite(pinsLeds[contadorDeApertos - 1], HIGH);
      } else {
        contadorDeApertos = 9; // Limitado ao número de leds
      }
      tone(PIN_BUZZER, 1000, 50);
      delay(250); // Debounce
    }

    // Detecção de botão de submissão
    if (digitalRead(PIN_BTN_SUBMIT) == LOW) {
      respondeu = true;
      tone(PIN_BUZZER, 1500, 100);
      delay(250); // Debounce
    }

    delay(10);
  }

  lcd.clear();
  totalPartidas++;
  float tempoReacao = (float)(millis() - tempoInicioResposta) / 1000.0;

  // 4. Verificação do Resultado
  if (timedOut) {
    statusJogo = "Tempo Esgotado!";
    comboStreak = 0;

    lcd.print("TEMPO ESGOTADO!");
    lcd.setCursor(0, 1);
    lcd.print("Era: ");
    lcd.print(numero);

    // Som de timeout
    tone(PIN_BUZZER, 200, 200);
    nonBlockingDelay(250);
    tone(PIN_BUZZER, 200, 200);

    adicionarHistorico(level, numero, 0, (float)tempoLimite / 1000.0, false,
                       true);
    nonBlockingDelay(3000);

  } else if (contadorDeApertos == numero) {
    statusJogo = "Acertou!";
    totalAcertos++;
    tempoSomaReacao += tempoReacao;
    comboStreak++;

    // Sistema de Pontuação e Multiplicador de Combo
    int pontosGanhos = 10 * comboStreak;
    score += pontosGanhos;
    xp += 10;

    if (score > highScore) {
      highScore = score;
    }

    lcd.print("ACERTOU!");
    lcd.setCursor(0, 1);
    lcd.print("+");
    lcd.print(pontosGanhos);
    lcd.print(" pts! Combo x");
    lcd.print(comboStreak);

    // Melodia de Acerto (Gamificação)
    tone(PIN_BUZZER, 1319, 100); // E6
    nonBlockingDelay(120);
    tone(PIN_BUZZER, 1568, 100); // G6
    nonBlockingDelay(120);
    tone(PIN_BUZZER, 1976, 150); // B6

    adicionarHistorico(level, numero, contadorDeApertos, tempoReacao, true,
                       false);
    nonBlockingDelay(2000);

    // Mecânica de Progressão de Nível
    if (xp >= 30) {
      if (level < 5) {
        level++;
        xp = 0;
        if (level > nivelMaximo) {
          nivelMaximo = level;
        }

        statusJogo = "Subiu de Nível!";
        lcd.clear();
        lcd.print("SUBIU DE NIVEL!");
        lcd.setCursor(0, 1);
        lcd.print("Nivel atual: ");
        lcd.print(level);

        // Melodia de subida de nível
        tone(PIN_BUZZER, 523, 100); // C5
        nonBlockingDelay(120);
        tone(PIN_BUZZER, 659, 100); // E5
        nonBlockingDelay(120);
        tone(PIN_BUZZER, 784, 100); // G5
        nonBlockingDelay(120);
        tone(PIN_BUZZER, 1047, 250); // C6

        nonBlockingDelay(2500);
      } else {
        xp = 30; // Nível máximo atingido
      }
    }

  } else {
    statusJogo = "Errou!";
    comboStreak = 0;

    lcd.print("ERROU!");
    lcd.setCursor(0, 1);
    lcd.print("Era: ");
    lcd.print(numero);
    lcd.print(" | Deu: ");
    lcd.print(contadorDeApertos);

    // Som de erro
    tone(PIN_BUZZER, 500, 300);
    nonBlockingDelay(350);
    tone(PIN_BUZZER, 300, 400);

    adicionarHistorico(level, numero, contadorDeApertos, tempoReacao, false,
                       false);
    nonBlockingDelay(3000);
  }

  // 5. Integração com Servidor em Nuvem (Simulação via POST Mock)
  if (WiFi.status() == WL_CONNECTED) {
    statusJogo = "Enviando dados...";
    HTTPClient http;
    http.begin("http://httpbin.org/post"); // Mock server público
    http.addHeader("Content-Type", "application/json");

    String jsonPayload =
        "{\"partida\":" + String(totalPartidas) +
        ",\"nivel\":" + String(level) + ",\"numeroAlvo\":" + String(numero) +
        ",\"numeroPalpite\":" + String(contadorDeApertos) +
        ",\"tempoReacao\":" + String(tempoReacao, 2) + ",\"acertou\":" +
        String(contadorDeApertos == numero ? "true" : "false") +
        ",\"timeout\":" + String(timedOut ? "true" : "false") +
        ",\"score\":" + String(score) + "}";

    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
      Serial.printf("Dados sincronizados com a Nuvem. Codigo: %d\n",
                    httpResponseCode);
    } else {
      Serial.printf("Falha na sincronizacao de dados: %s\n",
                    http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  }

  // Salva as estatísticas atualizadas na memória flash
  preferences.putInt("highScore", highScore);
  preferences.putInt("totalPartidas", totalPartidas);
  preferences.putInt("totalAcertos", totalAcertos);
  preferences.putInt("nivelMaximo", nivelMaximo);
  preferences.putFloat("tempoSomaReacao", tempoSomaReacao);

  // Reset de leds para iniciar a próxima rodada
  for (int i = 0; i < 9; i++) {
    digitalWrite(pinsLeds[i], LOW);
  }

  statusJogo = "Aguardando...";
  nonBlockingDelay(1000);
}