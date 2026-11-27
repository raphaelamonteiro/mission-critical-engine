# Dataset `telemetry_log.csv` — Mission Critical Engine

**Formato:** `seq_id,sensor_id,timestamp_ms,value,expected_status` (sem header, 130 linhas)
**Sensores (rodízio por `seq_id mod 3`):** 101 = temperatura (°C, nominal 22-28), 102 = tensão da bateria (V, nominal 24-28), 103 = pressão do tanque (bar, nominal 40-45)
**Timestamp base:** `T0 = 1699990000000` ms. Cadência nominal: 100 ms (10 Hz). Valores são inteiros em unidade de engenharia.
**Numeração de linhas abaixo:** linha do arquivo, 1-indexada.

## 1. Contrato do harness

O CSV sozinho não é suficiente para os testes. O harness deve garantir:

1. **CRC32 não está no CSV.** O harness monta o frame `(seq_id, sensor_id, timestamp_ms, value)`, calcula o CRC32 e, **somente para linhas `CORRUPTED`**, inverte 1 bit depois de calcular o CRC. Sugestão determinística: bit `seq_id % 32` do CRC. Sem isso, a linha com `9999` passaria no CRC (o valor absurdo estaria "corretamente" protegido).
2. **`expected_status` é o oráculo do teste, não entrada da engine.** Só o injetor de falhas e o comparador final o leem. Se a engine ler esse campo, a demo prova nada.
3. **Referência de timeout:** `atraso = max_timestamp_visto - timestamp_ms`. Se `atraso > 500`, vai para a DLQ.
4. **Ordem dos checks:** CRC → dedup → timeout → enqueue. Registre o `seq_id` na janela de dedup só **depois** de passar no CRC; senão uma retransmissão limpa de um pacote corrompido seria descartada por engano.
5. **Janela de dedup >= 5.** A maior distância entre uma duplicata e o `seq_id` mais recente é 4 (linha 31).
6. **Duplicatas não são DLQ:** são descartadas silenciosamente (premissa; ajuste se a sua engine fizer diferente).

## 2. Fases

| Fase | Linhas | seq_id | Conteúdo |
|------|--------|--------|----------|
| 1 Nominal | 1-30 | 1-30 | 30x OK, 100 ms de passo |
| 2 Idempotência | 31-40 | 26-30 (dup), 31-35 (novos) | 5x DUPLICATE + 5x OK de retomada |
| 3 Tempestade solar | 41-60 | 36-55 | 15x OK + 5x TIMEOUT |
| 4 Corrupção | 61-80 | 56-75 | 14x OK + 6x CORRUPTED |
| 5 Saturação | 81-130 | 76-125 | 50x OK, passo de 1 ms |

Totais: **OK 114 · DUPLICATE 5 · TIMEOUT 5 · CORRUPTED 6 = 130**.

## 3. Roteiro das anomalias, linha por linha

### Fase 2 — Idempotência (linhas 31-35)
Retransmissão em rajada dos 5 últimos frames da Fase 1, com timestamp e valor originais.
Todas têm atraso <= 400 ms, então **nenhuma é TIMEOUT em qualquer ordem de checks**.

| Linha | seq_id | Atraso vs. max ts | Fala sugerida |
|-------|--------|-------------------|---------------|
| 31 | 26 | 400 ms | "A DSN perdeu o ACK e reenviou. A engine reconhece o seq 26 e descarta." |
| 32 | 27 | 300 ms | "Mesmo frame, mesmo CRC: idempotência pura." |
| 33 | 28 | 200 ms | "Sem a janela deslizante, isso viraria leitura duplicada no controle térmico." |
| 34 | 29 | 100 ms | "Janela precisa cobrir pelo menos 5 posições." |
| 35 | 30 | 0 ms | "Duplicata imediata: o caso mais comum." |

Linhas 36-40 (seq 31-35) retomam o fluxo normal, provando que o dedup não bloqueou pacotes legítimos.

### Fase 3 — Tempestade solar / timeout (5 linhas)
Frames com timestamp bem anterior ao maior já visto, simulando atraso de rede de 1-2 s.

| Linha | seq_id | Sensor | ts efetivo (T0+) | max ts visto (T0+) | Atraso | Fala sugerida |
|-------|--------|--------|------------------|--------------------|--------|---------------|
| 44 | 39 | 103 | 2700 | 3700 | 1000 ms | "Primeiro pacote atrasado: 1 s de latência, o dobro do limite." |
| 48 | 43 | 101 | 2900 | 4000 | 1100 ms | "A leitura de temperatura chegou velha. Agir sobre ela seria perigoso." |
| 52 | 47 | 102 | 2950 | 4300 | 1350 ms | "Pacote de bateria atrasado: dado obsoleto não pode alimentar decisão." |
| 56 | 51 | 103 | 2800 | 4600 | 1800 ms | "Quase 2 s: a tempestade está piorando o link." |
| 59 | 54 | 103 | 3050 | 4800 | 1750 ms | "Mais um. A DLQ guarda para análise forense, não descarta." |

As linhas intercaladas (41-43, 45-47, 49-51, 53-55, 57-58, 60) são OK e mantêm a cadência de 100 ms.

### Fase 4 — Corrupção (6 linhas)
O harness inverte 1 bit no CRC de cada uma. Há dois tipos, e a diferença é o ponto didático da fase.

| Linha | seq_id | Sensor | Valor | Tipo | Fala sugerida |
|-------|--------|--------|-------|------|---------------|
| 63 | 58 | 101 | 9999 | Absurdo | "Temperatura de 9999 °C. Um range check pegaria, mas o CRC pega primeiro." |
| 67 | 62 | 102 | 65535 | Absurdo (uint16 max) | "Todos os bits em 1: assinatura clássica de SEU." |
| 71 | 66 | 103 | 44 | **Silenciosa** | "Valor perfeitamente plausível. Nenhuma validação de faixa detectaria. Só o CRC sabe." |
| 76 | 71 | 102 | -9999 | Absurdo | "Bit de sinal invertido." |
| 78 | 73 | 101 | 27 | **Silenciosa** | "De novo: dado errado com cara de dado certo. É o caso perigoso." |
| 80 | 75 | 103 | 32767 | Absurdo (int16 max) | "Fechamos a fase: 6 corrompidos, 0 chegam ao buffer." |

### Fase 5 — Saturação (linhas 81-130)
Timestamps com 1 ms de passo (1000 pacotes/s contra 10 pacotes/s antes). Todos válidos (`OK`) no nível de protocolo. O resultado depende do consumidor (seção 4).

## 4. Backpressure: configuração obrigatória do consumidor

**Problema aritmético:** 50 linhas < capacidade 64. Com buffer vazio, a rajada sozinha não estoura nada.
Para a demo, **pause o consumidor (sem `pop`) a partir da linha 41** (a tempestade também trava o lado terra).

Cálculo com essa configuração:

| Etapa | Pacotes que entram no buffer | Acumulado |
|-------|------------------------------|-----------|
| Fase 3 (linhas 41-60) | 15 | 15 |
| Fase 4 (linhas 61-80) | 14 | 29 |
| Fase 5, linhas 81-115 | 35 | **64 (cheio)** |
| Fase 5, linhas 116-130 | 0 (rejeitados) | 64 |

**Esperado:** linhas **116-130** (seq 111-125, 15 pacotes) rejeitadas por buffer cheio. A primeira rejeição ocorre na linha 116.

Outros pontos de pausa, úteis como testes de borda:

| Pausa a partir de | Rejeições | Observação |
|-------------------|-----------|------------|
| Linha 41 | 15 | Cenário da demo |
| Linha 61 | 0 | Buffer exatamente 64/64. Se a sua implementação sacrifica 1 slot (convenção head==tail), haverá 1 rejeição |
| Linha 81 | 0 | 50/64, sem estouro |

## 5. Orçamento da DLQ (capacidade 16)

- TIMEOUT (5) + CORRUPTED (6) = **11** entradas.
- Se pacotes rejeitados por buffer cheio **também** forem para a DLQ: 11 + 15 = 26. Entram só 5; **10 são perdidos por overflow da DLQ**.
- Decida antes da palestra se isso é bug ou feature: é um bom gancho para falar de "falha em cascata".

## 6. Expectativas finais (config da seção 4)

- Descartados por dedup: 5 · DLQ por timeout: 5 · DLQ por CRC: 6
- Enfileirados e retidos: 64 · Rejeitados por buffer cheio: 15
- Divergência esperada no comparador: as 15 linhas rejeitadas têm `expected_status=OK` mas resultado real `REJECTED_FULL`. O comparador deve tratar essa divergência como esperada.
