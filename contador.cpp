#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Update.h>

// Endereço I2C do LCD (32 é 0x20 decimal, mantido do projeto original do usuário)
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

const int pinsLeds[9] = { PIN_LED_1, PIN_LED_2, PIN_LED_3, PIN_LED_4, PIN_LED_5, PIN_LED_6, PIN_LED_7, PIN_LED_8, PIN_LED_9 };

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

// HTML/CSS/JS do Painel Web (Premium Dark Mode com Glassmorphism)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='pt-BR'>
<head>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
<title>Painel do Educador - Contador ESP32</title>
<link href='https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;800&display=swap' rel='stylesheet'>
<style>
	:root {
		--bg-primary: #0f172a;
		--bg-secondary: #1e293b;
		--accent: #6366f1;
		--accent-glow: rgba(99, 102, 241, 0.4);
		--success: #10b981;
		--danger: #ef4444;
		--warning: #f59e0b;
		--text-primary: #f8fafc;
		--text-secondary: #94a3b8;
	}
	* { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Outfit', sans-serif; }
	body {
		background: radial-gradient(circle at top right, #1e1b4b, var(--bg-primary));
		color: var(--text-primary);
		min-height: 100vh;
		padding: 2rem;
	}
	.container { max-width: 1100px; margin: 0 auto; }
	header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		margin-bottom: 2rem;
		border-bottom: 1px solid rgba(255,255,255,0.1);
		padding-bottom: 1rem;
	}
	h1 { font-size: 2rem; font-weight: 800; background: linear-gradient(135deg, #a5b4fc, #6366f1); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
	.status-badge {
		background: rgba(99, 102, 241, 0.2);
		border: 1px solid var(--accent);
		padding: 0.5rem 1rem;
		border-radius: 50px;
		font-size: 0.875rem;
		font-weight: 600;
		color: #a5b4fc;
		display: flex;
		align-items: center;
		gap: 0.5rem;
	}
	.status-dot {
		width: 8px;
		height: 8px;
		background-color: var(--success);
		border-radius: 50%;
		box-shadow: 0 0 10px var(--success);
		animation: pulse 1.5s infinite;
	}
	@keyframes pulse {
		0% { transform: scale(0.9); opacity: 0.6; }
		50% { transform: scale(1.2); opacity: 1; }
		100% { transform: scale(0.9); opacity: 0.6; }
	}
	.grid {
		display: grid;
		grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
		gap: 1.5rem;
		margin-bottom: 2rem;
	}
	.card {
		background: rgba(30, 41, 59, 0.4);
		backdrop-filter: blur(12px);
		-webkit-backdrop-filter: blur(12px);
		border: 1px solid rgba(255, 255, 255, 0.05);
		border-radius: 16px;
		padding: 1.5rem;
		transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
		box-shadow: 0 4px 30px rgba(0, 0, 0, 0.1);
	}
	.card:hover {
		transform: translateY(-5px);
		border-color: rgba(99, 102, 241, 0.3);
		box-shadow: 0 10px 30px rgba(99, 102, 241, 0.1);
	}
	.card-title {
		font-size: 0.875rem;
		color: var(--text-secondary);
		text-transform: uppercase;
		letter-spacing: 0.05em;
		margin-bottom: 0.5rem;
	}
	.card-value {
		font-size: 2.25rem;
		font-weight: 800;
		color: var(--text-primary);
	}
	.card-sub {
		font-size: 0.875rem;
		color: var(--text-secondary);
		margin-top: 0.25rem;
	}
	.main-section {
		display: grid;
		grid-template-columns: 2fr 1fr;
		gap: 1.5rem;
	}
	@media(max-width: 768px) {
		.main-section { grid-template-columns: 1fr; }
	}
	.table-container {
		overflow-x: auto;
	}
	table {
		width: 100%;
		border-collapse: collapse;
		text-align: left;
		margin-top: 1rem;
	}
	th {
		padding: 0.75rem 1rem;
		color: var(--text-secondary);
		font-weight: 600;
		border-bottom: 1px solid rgba(255,255,255,0.05);
	}
	td {
		padding: 1rem;
		border-bottom: 1px solid rgba(255,255,255,0.05);
	}
	tr:hover td {
		background: rgba(255,255,255,0.02);
	}
	.badge {
		padding: 0.25rem 0.5rem;
		border-radius: 4px;
		font-size: 0.75rem;
		font-weight: 600;
	}
	.badge-success { background: rgba(16, 185, 129, 0.2); color: #34d399; }
	.badge-danger { background: rgba(239, 68, 68, 0.2); color: #f87171; }
	.badge-warning { background: rgba(245, 158, 11, 0.2); color: #fbbf24; }
	
	.btn {
		background: var(--accent);
		color: #fff;
		border: none;
		padding: 0.75rem 1.5rem;
		border-radius: 8px;
		font-weight: 600;
		cursor: pointer;
		transition: all 0.2s ease;
		width: 100%;
		text-align: center;
		text-decoration: none;
		display: inline-block;
	}
	.btn:hover {
		background: #4f46e5;
		box-shadow: 0 0 15px var(--accent-glow);
	}
	.btn-danger {
		background: var(--danger);
	}
	.btn-danger:hover {
		background: #dc2626;
		box-shadow: 0 0 15px rgba(239, 68, 68, 0.4);
	}
	.form-group {
		margin-bottom: 1rem;
	}
	label {
		display: block;
		font-size: 0.875rem;
		color: var(--text-secondary);
		margin-bottom: 0.5rem;
	}
	select {
		width: 100%;
		padding: 0.75rem;
		background: rgba(15, 23, 42, 0.6);
		border: 1px solid rgba(255,255,255,0.1);
		border-radius: 8px;
		color: var(--text-primary);
		outline: none;
	}
	select:focus {
		border-color: var(--accent);
	}
	.ota-container {
		border: 2px dashed rgba(255,255,255,0.1);
		padding: 2rem;
		border-radius: 12px;
		text-align: center;
		cursor: pointer;
		margin-top: 1rem;
	}
	.ota-container:hover {
		border-color: var(--accent);
	}
</style>
</head>
<body>
<div class='container'>
	<header>
		<h1>Painel do Educador - Jogo de Contagem</h1>
		<div class='status-badge'>
			<div class='status-dot'></div>
			<span id='game-status'>Carregando...</span>
		</div>
	</header>
	
	<div class='grid'>
		<div class='card'>
			<div class='card-title'>Nível Atual</div>
			<div class='card-value' id='val-level'>-</div>
			<div class='card-sub' id='val-xp'>XP: -</div>
		</div>
		<div class='card'>
			<div class='card-title'>Pontuação Atual</div>
			<div class='card-value' id='val-score'>-</div>
			<div class='card-sub' id='val-high-score'>Recorde: -</div>
		</div>
		<div class='card'>
			<div class='card-title'>Taxa de Acertos</div>
			<div class='card-value' id='val-accuracy'>-</div>
			<div class='card-sub' id='val-matches'>Partidas: -</div>
		</div>
		<div class='card'>
			<div class='card-title'>Tempo de Resposta</div>
			<div class='card-value' id='val-reaction'>-</div>
			<div class='card-sub'>Média por acerto</div>
		</div>
	</div>
	
	<div class='main-section'>
		<div class='card'>
			<h2>Histórico de Partidas Recentes</h2>
			<div class='table-container'>
				<table>
					<thead>
						<tr>
							<th>Partida</th>
							<th>Nível</th>
							<th>Alvo</th>
							<th>Palpite</th>
							<th>Tempo</th>
							<th>Resultado</th>
						</tr>
					</thead>
					<tbody id='history-table-body'>
						<tr><td colspan='6' style='text-align:center;'>Aguardando dados...</td></tr>
					</tbody>
				</table>
			</div>
		</div>
		
		<div style='display: flex; flex-direction: column; gap: 1.5rem;'>
			<div class='card'>
				<h2>Configurações</h2>
				<div style='margin-top: 1rem;'>
					<div class='form-group'>
						<label>Forçar Nível do Jogo</label>
						<select id='set-level' onchange='updateConfig()'>
							<option value='1'>Nível 1 (1 a 5) - Memorização: 8s</option>
							<option value='2'>Nível 2 (1 to 7) - Memorização: 6s</option>
							<option value='3'>Nível 3 (1 to 9) - Memorização: 5s</option>
							<option value='4'>Nível 4 (1 to 9) - Memorização: 3s</option>
							<option value='5'>Nível 5 (1 to 9) - Memorização: 2s</option>
						</select>
					</div>
					<button class='btn btn-danger' style='margin-top:0.5rem;' onclick='resetStats()'>Zerar Estatísticas</button>
				</div>
			</div>
			
			<div class='card'>
				<h2>Atualização OTA (Wireless)</h2>
				<p style='font-size:0.875rem; color:var(--text-secondary); margin-top:0.5rem;'>Suba um firmware (.bin) compilado para atualizar o ESP32 remotamente.</p>
				<form method='POST' action='/update' enctype='multipart/form-data'>
					<div class='ota-container' onclick='document.getElementById("ota-file").click()'>
						<span id='ota-filename'>Clique para escolher o arquivo</span>
						<input type='file' id='ota-file' name='update' style='display:none;' onchange='updateFileName(this)'>
					</div>
					<button type='submit' class='btn' style='margin-top: 1rem;'>Atualizar Firmware</button>
				</form>
			</div>
		</div>
	</div>
</div>

<script>
	function fetchStats() {
		fetch('/api/stats')
			.then(r => r.json())
			.then(data => {
				document.getElementById('val-level').innerText = data.level;
				document.getElementById('val-xp').innerText = 'XP: ' + data.xp + ' / 30';
				document.getElementById('val-score').innerText = data.score;
				document.getElementById('val-high-score').innerText = 'Recorde: ' + data.highScore;
				document.getElementById('val-accuracy').innerText = data.taxaAcertos.toFixed(1) + '%';
				document.getElementById('val-matches').innerText = 'Total: ' + data.totalPartidas + ' (Acertos: ' + data.totalAcertos + ')';
				document.getElementById('val-reaction').innerText = data.tempoMedioReacao.toFixed(2) + 's';
				document.getElementById('game-status').innerText = data.status;
				document.getElementById('set-level').value = data.level;
				
				let tbody = document.getElementById('history-table-body');
				if (data.historico && data.historico.length > 0) {
					tbody.innerHTML = '';
					data.historico.forEach((h) => {
						let resBadge = h.acertou ? "<span class='badge badge-success'>ACERTOU</span>" : (h.timeout ? "<span class='badge badge-warning'>TIMEOUT</span>" : "<span class='badge badge-danger'>ERROU</span>");
						tbody.innerHTML += `<tr>
							<td>#${h.partida}</td>
							<td>Lvl ${h.nivel}</td>
							<td>${h.numeroAlvo}</td>
							<td>${h.numeroPalpite}</td>
							<td>${h.tempoReacao.toFixed(2)}s</td>
							<td>${resBadge}</td>
						</tr>`;
					});
				} else {
					tbody.innerHTML = "<tr><td colspan='6' style='text-align:center; color:var(--text-secondary);'>Nenhuma partida registrada ainda</td></tr>";
				}
			});
	}
	
	function updateFileName(input) {
		if(input.files && input.files[0]) {
			document.getElementById('ota-filename').innerText = input.files[0].name;
		}
	}
	
	function updateConfig() {
		let lvl = document.getElementById('set-level').value;
		fetch('/api/config', {
			method: 'POST',
			headers: {'Content-Type': 'application/x-www-form-urlencoded'},
			body: 'level=' + lvl
		}).then(() => fetchStats());
	}

	function resetStats() {
		if(confirm('Tem certeza que deseja zerar todas as estatísticas de acerto e pontuações?')) {
			fetch('/api/config', {
				method: 'POST',
				headers: {'Content-Type': 'application/x-www-form-urlencoded'},
				body: 'reset=1'
			}).then(() => fetchStats());
		}
	}
	
	setInterval(fetchStats, 2000);
	fetchStats();
</script>
</body>
</html>
)rawliteral";

// Delay não-bloqueante para manter o Web Server responsivo
void nonBlockingDelay(unsigned long ms) {
	unsigned long start = millis();
	while (millis() - start < ms) {
		server.handleClient();
		delay(1);
	}
}

// Adiciona uma partida ao histórico local
void adicionarHistorico(int nivel, int target, int guess, float time, bool hit, bool timeout) {
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

void setup()
{
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
	server.on("/", HTTP_GET, []() {
		server.send(200, "text/html", index_html);
	});
	
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
			json += "\"numeroPalpite\":" + String(historicoPartidas[i].numeroPalpite) + ",";
			json += "\"tempoReacao\":" + String(historicoPartidas[i].tempoReacao, 2) + ",";
			json += "\"acertou\":" + String(historicoPartidas[i].acertou ? "true" : "false") + ",";
			json += "\"timeout\":" + String(historicoPartidas[i].timeout ? "true" : "false");
			json += "}";
			if (i < historicoCount - 1) json += ",";
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
	server.on("/update", HTTP_POST, []() {
		server.sendHeader("Connection", "close");
		server.send(200, "text/html", Update.hasError() ? "Falha na atualizacao!" : "<h3>Atualizacao efetuada com sucesso! Reiniciando o ESP32...</h3>");
		delay(1000);
		ESP.restart();
	}, []() {
		HTTPUpload& upload = server.upload();
		if (upload.status == UPLOAD_FILE_START) {
			Serial.printf("Update: %s\n", upload.filename.c_str());
			if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { 
				Update.printError(Serial);
			}
		} else if (upload.status == UPLOAD_FILE_WRITE) {
			if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
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

void loop()
{
	// 1. Definição da dificuldade com base no nível
	int maxNumero = 5;
	unsigned long tempoMemorizar = 8000;
	unsigned long tempoLimite = 15000;
	
	switch(level) {
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
		
		adicionarHistorico(level, numero, 0, (float)tempoLimite/1000.0, false, true);
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
		
		adicionarHistorico(level, numero, contadorDeApertos, tempoReacao, true, false);
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
		
		adicionarHistorico(level, numero, contadorDeApertos, tempoReacao, false, false);
		nonBlockingDelay(3000);
	}
	
	// 5. Integração com Servidor em Nuvem (Simulação via POST Mock)
	if (WiFi.status() == WL_CONNECTED) {
		statusJogo = "Enviando dados...";
		HTTPClient http;
		http.begin("http://httpbin.org/post"); // Mock server público
		http.addHeader("Content-Type", "application/json");
		
		String jsonPayload = "{\"partida\":" + String(totalPartidas) +
							 ",\"nivel\":" + String(level) +
							 ",\"numeroAlvo\":" + String(numero) +
							 ",\"numeroPalpite\":" + String(contadorDeApertos) +
							 ",\"tempoReacao\":" + String(tempoReacao, 2) +
							 ",\"acertou\":" + String(contadorDeApertos == numero ? "true" : "false") +
							 ",\"timeout\":" + String(timedOut ? "true" : "false") +
							 ",\"score\":" + String(score) + "}";
							 
		int httpResponseCode = http.POST(jsonPayload);
		if (httpResponseCode > 0) {
			Serial.printf("Dados sincronizados com a Nuvem. Codigo: %d\n", httpResponseCode);
		} else {
			Serial.printf("Falha na sincronizacao de dados: %s\n", http.errorToString(httpResponseCode).c_str());
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