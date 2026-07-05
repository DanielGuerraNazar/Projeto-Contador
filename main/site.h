#ifndef SITE_H
#define SITE_H

#include <Arduino.h>

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

#endif
