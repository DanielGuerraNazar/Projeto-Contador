# Regras de Versionamento do Projeto

Sempre que o usuário disser o comando **"Agente, suba a nova versão"**, adote a seguinte regra estrita de versionamento:

1. **Sincronizar Antes de Tudo (Passo Zero)**:
   - Antes de gerar qualquer versão, execute um `git pull origin main` (ou na branch ativa) para garantir que o workspace local tenha todas as atualizações e tags que outros colaboradores enviaram para o GitHub.

2. **Identificar a última versão**:
   - Verifique qual foi a última tag ou commit de versão criado no repositório local.
   - Se não houver nenhum, comece na **1.0**.
   - Se houver (ex: 1.0), incremente sempre o número inteiro para a próxima versão (**2.0**, **3.0**, etc.).

3. **Capturar a data atual**:
   - Obtenha a data atual do sistema no formato `DD-MM-AAAA`.

4. **Gerar o resumo das alterações**:
   - Faça uma análise rápida dos arquivos modificados no workspace (`git diff`).
   - Monte um resumo ultra-conciso (em apenas uma linha entre parênteses) com as principais modificações que foram feitas no código.

5. **Executar os comandos do Git**:
   - Execute `git add .` para incluir todas as alterações pendentes.
   - Faça o commit usando exatamente o padrão:
     `[Versão X.0] - DD-MM-AAAA (Resumo conciso das alterações feito pela IA)`
   - Crie uma tag local com o número da versão para controle do repositório (ex: `git tag vX.0`).
   - Execute `git push origin main --tags` para fazer o upload completo do código, do commit e da nova tag para o GitHub.
