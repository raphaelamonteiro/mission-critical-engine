# Demo: The Cosmic Ray (O Raio Cósmico)

## 1. Execução

Execute o projeto utilizando:

bash

```bash
make run
```

## 2. Estado inicial: Pacote íntegro

Explique à audiência:

> "O pacote de telemetria chega intacto. O sistema calcula seu CRC32, compara com a assinatura esperada e confirma que os dados estão íntegros. Por isso, recebemos o status `PKT_OK`."

O que acontece tecnicamente:

- O pacote é criado.
- O CRC32 é calculado.
- A integridade é validada.
- O pacote é aceito pelo sistema.

## 3. Simulação: O raio cósmico

Ação: modifique intencionalmente algum dado do pacote em `main.c` ou aguarde o mecanismo de simulação de corrupção executar.

Explique à audiência:

> "Agora vamos imaginar que um raio cósmico atingiu nossa nave e alterou um único bit da memória. Parece pouco, mas em um sistema crítico, um único bit pode fazer toda a diferença."

## 4. Resultado: Corrupção detectada

Explique à audiência:

> "O CRC32 identifica que os dados não correspondem mais à assinatura original. O sistema registra `PKT_CORRUPTED` e rejeita o pacote. O mais importante: o programa continua funcionando."

O que acontece tecnicamente:

- O conteúdo do pacote é alterado.
- O CRC32 é recalculado.
- A assinatura calculada diverge da original.
- O sistema identifica a corrupção.
- O pacote é rejeitado.

## 5. Mensagem principal da demonstração

> "Percebam que não precisamos esperar o sistema quebrar para descobrir que algo deu errado. Detectamos a corrupção, isolamos o dado inválido e continuamos operando."

Conexão com a palestra — Além do Try/Catch:

Não estamos simplesmente tratando uma exceção depois que o problema acontece. Estamos verificando continuamente a integridade dos dados e impedindo que informações corrompidas avancem pelo sistema.

A grande lição: um sistema resiliente não é aquele em que falhas nunca acontecem, mas aquele que consegue detectá-las e sobreviver a elas.
