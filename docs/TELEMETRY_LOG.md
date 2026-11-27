# Documentação do dataset `telemetry_log.csv`

**Formato:** `seq_id,sensor_id,timestamp_epoch,value` (sem header, 50 linhas)
**Sensores:** 101 = temperatura interna (°C), 102 = tensão da bateria (V), 103 = pressão do tanque (bar)
**Regra de geração:** sensores em rodízio (`seq_id mod 3`: 1→101, 2→102, 0→103).
Timestamp de pacote novo = `1699990000 + (seq_id - 1)`.

## Narrativa por fase

| Linhas | Fase | seq_id | Objetivo do teste |
|--------|------|--------|-------------------|
| 1-15   | Operação nominal | 1-15 | Baseline: caminho feliz, sem falhas |
| 16-25  | Retransmissões | 16-21 (novos) + 4 duplicatas | Idempotência / janela deslizante |
| 26-38  | Tempestade solar | 22-34 | Spikes, bit-flips e timestamps fora de ordem |
| 39-50  | Recuperação | 35-46 | Retorno gradual ao nominal |

Total: 46 `seq_id` únicos + 4 duplicatas = 50 linhas.

## Duplicatas (fase 2)

| Linha | seq_id duplicado | Maior seq já visto | Distância | Cópia idêntica de |
|-------|------------------|--------------------|-----------|-------------------|
| 17 | 14 | 16 | 2 | linha 14 |
| 19 | 15 | 17 | 2 | linha 15 |
| 22 | 19 | 19 | 0 | linha 21 (duplicata imediata) |
| 24 | 18 | 20 | 2 | linha 20 |

**Implicação:** a janela deslizante precisa ter tamanho >= 3 para descartar todas.
Uma janela de tamanho 2 deixaria passar as duplicatas de distância 2.

## Anomalias (fase 3)

| Linha | seq_id | Sensor | Valor | Tipo |
|-------|--------|--------|-------|------|
| 29 | 25 | 101 | 99   | Spike de temperatura (bit-flip simulado) |
| 31 | 27 | 103 | 255  | Overflow de payload (0xFF) |
| 32 | 28 | 101 | -128 | Underflow int8 (0x80) |
| 33 | 29 | 102 | 0    | Colapso de tensão |
| 35 | 31 | 101 | 97   | Spike residual |
| 36 | 32 | 102 | 29   | Fora da faixa nominal (leve) |
| 37 | 33 | 103 | 38   | Fora da faixa nominal (leve) |
| 38 | 34 | 101 | 31   | Fora da faixa nominal (leve) |

**Timestamps fora de ordem:**
- Linha 34 (seq 30): ts `1699990024`, esperado `1699990029` (regressão de 5 s)
- Linha 37 (seq 33): ts `1699990021`, esperado `1699990032` (regressão de 11 s)

## Resultado esperado no simulador

- 46 pacotes únicos processados, 4 duplicatas descartadas
- Pacotes das linhas 34 e 37: candidatos à Dead-Letter Queue (se o simulador usar o ts como referência de atraso)
- Linhas 29, 31, 32, 33, 35: devem ser sinalizadas como anômalas/corrompidas
- Fase 4: valores de volta às faixas nominais até o seq 46

## Limitações conhecidas (leia antes da demo)

1. **Não há coluna CRC32.** O formato de 4 colunas não carrega checksum. O simulador precisa
   calcular o CRC32 do payload na leitura e injetar bit-flips por conta própria. Neste dataset,
   o "bit-flip" é representado semanticamente (valores absurdos), não por corrupção real de bits.
2. **Timeout de 500 ms vs. timestamp em segundos.** A resolução de 1 s não consegue expressar
   atraso de 500 ms. O timeout só dispara se o simulador comparar o `timestamp_epoch` com seu
   relógio monotônico de chegada. As regressões das linhas 34 e 37 são o gatilho mais confiável.
3. **Ring buffer (64) não satura com 50 linhas.** Para demonstrar overflow, pause o consumidor
   ou repita o arquivo em loop.
