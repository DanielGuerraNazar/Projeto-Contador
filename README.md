# Projeto-Contador

## Visão Geral
Este projeto implementa um jogo educativo para crianças, com contagem de apertos, tempo de reação e pontuação. O sistema sincroniza dados de partidas para a nuvem e armazena estatísticas em memória flash.

## Funcionalidades Recentes
- **Sincronização em nuvem**: Dados da partida são enviados como JSON para um endpoint HTTP seguro.
- **Conversão de documentação**: O arquivo `antigravity.pdf` foi convertido para Markdown (`antigravity.md`).
- **Versionamento SemVer**: O script `commit_and_push.sh` agora gera tags no formato `vMAJOR.MINOR.PATCH` e cria um `CHANGELOG.md` automático.

## Como usar
1. Abra o projeto no Arduino IDE ou PlatformIO.
2. Compile e faça upload para o ESP32.
3. Ao finalizar uma partida, os dados são enviados automaticamente.
4. Para versionar e publicar, execute:
   ```sh
   ./commit_and_push.sh "Descrição das alterações"
   ```
   Opcionalmente, passe `major`, `minor` ou `patch` como segundo argumento para forçar o nível de versão.

## Estrutura do Repositório
- `contador.cpp` – Código-fonte principal.
- `commit_and_push.sh` – Script de versionamento e publicação.
- `antigravity.md` – Documento de proposta em Markdown (gerado a partir do PDF).
- `CHANGELOG.md` – Histórico de versões (gerado automaticamente).

## Contribuição
Abra *issues* para sugestões e envie *pull requests* seguindo o padrão de versionamento acima.
