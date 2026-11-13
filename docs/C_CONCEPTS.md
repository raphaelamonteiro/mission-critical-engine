# Conceitos de C e sua relação com o tema "Além do Try/Catch"

## 1. Conceitos fundamentais da linguagem C

### `#include` e `#ifndef` — Include Guards

O C não possui um sistema de importação mágico como outras linguagens. O `#include` funciona essencialmente copiando e colando o conteúdo de um arquivo no local da inclusão.

O `#ifndef` (Include Guard) impede que o mesmo arquivo seja incluído múltiplas vezes durante a compilação, evitando redefinições e conflitos.

**Relação com a palestra:** antes mesmo de executar o programa, precisamos garantir que sua estrutura seja consistente.

### Tipos de largura fixa (`uint32_t`)

Em sistemas críticos, não podemos depender de tamanhos implícitos de tipos como `int`, pois eles podem variar conforme a arquitetura e o compilador.

Utilizamos tipos de largura fixa, como `uint32_t`, para garantir que uma variável tenha exatamente 32 bits.

**Relação com a palestra:** previsibilidade é uma forma de segurança. Não podemos depender de comportamentos que variam entre plataformas.

### Ponteiros (`*`) e endereços (`&`)

Ponteiros armazenam endereços de memória.

No projeto, a função `integrity_compute_crc32` recebe `const void *data`, permitindo receber um endereço de memória genérico.

Ao passar `&my_packet`, entregamos à função o endereço do pacote para que ela possa acessar seus dados e calcular sua integridade.

O modificador `const` indica que a função não deve modificar os dados recebidos por esse ponteiro.

**Relação com a palestra:** trabalhar diretamente com memória exige cuidado. Um endereço incorreto ou uma leitura indevida pode comprometer todo o sistema.

### Structs — Organização dos dados

As `structs` permitem agrupar diferentes variáveis em uma única estrutura lógica.

No projeto, utilizamos a estrutura `TelemetryPacket` para representar um pacote de telemetria espacial e a estrutura `RingBuffer` para controlar o armazenamento circular desses pacotes.

**Relação com a palestra:** organizar os dados em estruturas bem definidas facilita o controle do estado do sistema e reduz a possibilidade de manipulação inconsistente.

### Operações bitwise (`^`, `>>`, `~`)

Operações bitwise manipulam diretamente os bits de um valor.

- `^`: XOR (OU exclusivo).
- `>>`: deslocamento de bits à direita.
- `~`: inversão dos bits.

Essas operações são fundamentais no algoritmo CRC32, que utiliza operações matemáticas sobre bits para calcular uma assinatura de integridade dos dados.

**Relação com a palestra:** a integridade não é presumida. Ela é verificada matematicamente.

### Aritmética Modular (`%`) — O ciclo do Ring Buffer

O operador `%` (módulo) retorna o resto de uma divisão. No Ring Buffer, ele permite que um índice volte automaticamente ao início do array quando alcançar sua capacidade máxima.

Exemplo:

c

```c
index = (index + 1) % CAPACIDADE;
```

Se a capacidade for 64, quando o índice chegar a 63, o próximo valor será 0.

Isso permite reutilizar posições de memória sem precisar deslocar os elementos do array.

**Relação com a palestra:** em sistemas críticos, precisamos controlar o uso da memória de maneira previsível. O Ring Buffer permite trabalhar com memória previamente alocada, evitando alocações dinâmicas constantes.

### Modificadores `const` e passagem por ponteiro

Em C, podemos passar estruturas para funções utilizando ponteiros.

Exemplo:

```c
void ring_buffer_push(RingBuffer *rb, const TelemetryPacket *packet);
```

Nesse caso:

- `RingBuffer *rb`: permite modificar o buffer original.
- `const TelemetryPacket *packet`: permite ler o pacote sem modificá-lo.

Dessa maneira, as funções conseguem operar diretamente sobre o buffer principal, sem precisar copiar toda a estrutura.

**Relação com a palestra:** controlar explicitamente quem pode modificar os dados é fundamental para manter a integridade e a previsibilidade do sistema.

### `_Static_assert` — Verificação em tempo de compilação

Permite estabelecer condições que precisam ser verdadeiras antes que o programa seja compilado.

No projeto, utilizamos:

```c
_Static_assert(sizeof(TelemetryPacket) == 20, "Invalid packet size");
```

Isso garante que o pacote tenha exatamente 20 bytes. Se sua estrutura mudar e deixar de respeitar esse tamanho, a compilação falhará.

**Relação com a palestra:** por que esperar um erro acontecer em produção se podemos impedir que o programa seja compilado com uma estrutura inválida?

É uma primeira linha de defesa, anterior até mesmo à execução.

---

## 2. Organização dos arquivos do projeto

### `config.h` — As regras do sistema

Centraliza as configurações e os parâmetros utilizados pelo projeto.

Define:

- Capacidade da órbita: 64 pacotes.
- Tempo limite: 500 ms.
- Semente do caos: `0xC0FFEE`.
- Capacidade do Ring Buffer, caso seja definida separadamente da capacidade da órbita.

**Papel na arquitetura:** estabelece os parâmetros que determinam como o sistema deve funcionar.

### `telemetry.h` — O contrato de dados

Define a estrutura `TelemetryPacket`, que representa um pacote de telemetria espacial.

O pacote possui cinco campos e ocupa exatamente 20 bytes.

**Papel na arquitetura:** estabelece o contrato de dados. Todos os componentes precisam concordar sobre o formato e o tamanho do pacote.

### `integrity.h` e `integrity.c` — O fiscal da integridade

Implementam a lógica de verificação CRC32.

A função recebe os dados, calcula uma assinatura matemática e permite detectar alterações no conteúdo do pacote.

**Papel na arquitetura:** fiscaliza a integridade dos dados e identifica possíveis corrupções durante seu processamento ou transporte.

### `ring_buffer.h` e `ring_buffer.c` — O sistema de armazenamento circular

Implementam o Ring Buffer, uma estrutura de dados que armazena pacotes em um array de tamanho fixo e reutiliza suas posições de maneira circular.

A estrutura `RingBuffer` possui:

- `buffer`: array que armazena os pacotes de telemetria.
- `head`: índice da próxima posição de escrita.
- `tail`: índice da próxima posição de leitura.
- `count`: quantidade de elementos armazenados, caso essa estratégia seja utilizada para distinguir buffer cheio de buffer vazio.

Exemplo conceitual:

```c
typedef struct {
    TelemetryPacket buffer[CAPACIDADE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} RingBuffer;
```

Responsabilidades:

- Inicializar o buffer.
- Inserir novos pacotes (`push`).
- Remover ou recuperar pacotes (`pop`).
- Controlar os índices `head` e `tail`.
- Detectar quando o buffer está cheio ou vazio.

**Papel na arquitetura:** gerencia o fluxo de pacotes de telemetria de maneira previsível, utilizando memória previamente alocada e evitando deslocamentos desnecessários de dados.

### `main.c` — O cérebro do sistema

Coordena a execução do programa e organiza os experimentos.

É responsável por:

- Criar os pacotes de telemetria.
- Inicializar o Ring Buffer.
- Inserir e recuperar pacotes.
- Calcular suas assinaturas CRC32.
- Simular uma interferência, representando um raio cósmico.
- Verificar novamente a integridade.
- Demonstrar a detecção e rejeição do pacote corrompido.
- Demonstrar o comportamento do sistema quando o buffer está cheio ou vazio.

**Papel na arquitetura:** orquestra os experimentos e demonstra que os mecanismos de proteção funcionam.

### `Makefile` — A esteira de compilação

Automatiza o processo de compilação do projeto.

Centraliza os comandos necessários para gerar o executável, facilitando a reprodução dos testes e evitando comandos manuais repetitivos.

**Papel na arquitetura:** garante um processo de build organizado e reproduzível.

---

## 3. O que provamos na prática?

Com a introdução do Ring Buffer e do CRC32, demonstramos duas propriedades importantes de um sistema confiável:

### Integridade dos dados

O CRC32 permite identificar alterações nos dados dos pacotes.

O experimento simula uma alteração nos dados de telemetria, calcula novamente o CRC32 e compara o resultado com a assinatura original.

Quando os valores divergem, identificamos que o conteúdo foi alterado e podemos rejeitar o pacote.

### Gerenciamento controlado de memória

O Ring Buffer armazena e organiza os pacotes em um espaço limitado e reutilizável.

O sistema consegue receber vários pacotes, armazená-los em uma estrutura circular e recuperá-los sem precisar deslocar os elementos do array.

Também conseguimos demonstrar:

- Reutilização de posições de memória.
- Buffer cheio.
- Buffer vazio.
- Controle de leitura e escrita.
- Prevenção de acesso fora dos limites do array.

Isso evita que pacotes sejam armazenados ou processados de maneira descontrolada.

**Importante:** o CRC32 detecta muitas formas de corrupção acidental, mas não garante a detecção de toda alteração possível nem oferece proteção criptográfica contra adulteração intencional. Da mesma forma, o Ring Buffer precisa definir explicitamente o comportamento quando está cheio, como rejeitar novos pacotes ou sobrescrever os mais antigos.

---

## 4. Conexão com o tema: Além do Try/Catch

Em linguagens com mecanismos de exceção, é comum pensar na recuperação de erros a partir de estruturas como `try/catch`.

Mas em C, especialmente em sistemas críticos, a estratégia de confiabilidade precisa começar muito antes de um erro acontecer.

Nosso projeto demonstra quatro camadas de proteção:

1. **Prevenção em tempo de compilação:** usamos `_Static_assert` e tipos de largura fixa para garantir estruturas previsíveis e evitar inconsistências.
2. **Gerenciamento controlado de memória:** utilizamos o Ring Buffer para controlar os limites de leitura e escrita e reutilizar posições de memória.
3. **Verificação em tempo de execução:** utilizamos CRC32 para detectar corrupção dos dados.
4. **Tratamento da falha:** rejeitamos pacotes corrompidos e identificamos situações como buffer cheio ou vazio, evitando que dados inválidos avancem pelo sistema.

A ideia central é que confiabilidade não significa apenas tratar erros depois que acontecem. Significa projetar o sistema para prevenir, detectar e conter falhas.

---

## 5. Pilar 1 da palestra: Detectar antes que seja tarde

A principal lição do experimento é:

> Não basta saber tratar uma falha. Precisamos projetar sistemas capazes de detectar que algo está errado antes que essa falha se transforme em um problema maior.

O projeto demonstra que confiabilidade não depende exclusivamente de mecanismos de tratamento de erros.

Ela começa na definição dos dados, passa pelas garantias da compilação, continua no gerenciamento controlado da memória, avança para a validação da integridade durante a execução e termina com uma decisão segura diante de uma falha.

Esse é o primeiro pilar da palestra **"Além do Try/Catch"**: construir sistemas que não apenas reagem aos erros, mas que são projetados para identificá-los, contê-los e impedir que se propaguem.

---

## 6. A evolução da mensagem da palestra

Com CRC32 e Ring Buffer, conseguimos contar uma história completa sobre confiabilidade em C:

- `_Static_assert`: impede estruturas inválidas antes da execução.
- Tipos de largura fixa: garantem previsibilidade na representação dos dados.
- Ponteiros e structs: permitem controlar como os dados são organizados e acessados.
- Ring Buffer: controla onde os dados ficam armazenados e como circulam pela memória.
- Aritmética modular: permite reutilizar posições do buffer sem ultrapassar seus limites.
- CRC32: verifica se os dados foram corrompidos.
- Tratamento de falhas: rejeita dados inválidos e identifica condições inesperadas.

> Não basta detectar que algo deu errado. Precisamos controlar como os dados entram, onde são armazenados, como são validados e o que acontece quando alguma dessas etapas falha.

**A grande mensagem:** um sistema resiliente não é aquele em que falhas nunca acontecem, mas aquele que possui mecanismos para prevenir, detectar, conter e sobreviver a elas.

## Uma observação técnica importante

Na implementação, recomendo manter o `count` no Ring Buffer. Com `head == tail`, sem contador ou outra estratégia equivalente, não conseguimos distinguir com segurança um buffer vazio de um buffer cheio.

Também vale definir uma política explícita de overflow: para essa demonstração, eu começaria rejeitando novos pacotes quando o buffer estiver cheio, registrando um erro como `BUFFER_FULL`. Isso reforça muito bem a mensagem de que uma falha detectada e controlada é melhor do que uma falha silenciosa.

## Modular Arithmetic and Pointers in Critical Systems

- **The Modulo Operator (`%`)**: Used to wrap the `head` and `tail` indices. When `head` reaches `CAPACITY`, `(head + 1) % CAPACITY` resets it to `0`. This creates a circular buffer without shifting array elements, maintaining O(1) performance.
- **The Arrow Operator (`->`)**: When passing a struct by pointer (`RingBuffer *rb`), the `->` operator accesses its members (`rb->head`). This avoids copying the entire struct (which contains 64 packets) onto the stack for every function call, saving memory and CPU cycles.
- **Early Returns**: Instead of nested `if/else` blocks, the code checks for error conditions (full/empty) at the top of the function and returns immediately. This flattens the code, making it easier to read and less prone to logic errors in critical paths.

Módulo Responsabilidade única Conceito de C que ele demonstra
telemetry Modelo de dados (POD) struct, enum, \_Static_assert, packing
ring_buffer Isolamento produtor/consumidor ponteiros, aritmética modular (% cap), \_Atomic
integrity Detecção de corrupção CRC32, uint32_t, operações bit-a-bit
idempotency Evitar reprocessamento janela deslizante, bool[] ou bitmap
fault_injector Simular caos PRNG (rand_r/xorshift), memcpy controlado
timeout Detectar atrasos clock_gettime, timespec, deadlines
engine Pipeline ponteiros para função (callbacks), máquina de estados
tui Feedback visual \033[...m, cursor, fflush

telemetry.h + config.h — o "contrato" de dados que todos os módulos compartilham. Sem isso, os outros módulos ficam vagos.

integrity — CRC32 é pequeno, autocontido e fácil de testar (main temporário ou teste embutido).

ring_buffer — coração do isolamento. Aqui entramos em ponteiros e aritmética modular.

idempotency — depende do campo seq_id do pacote.

fault_injector — plugável no caminho de ingestão.

timeout — deadline por pacote + política de degradação.

engine — cola tudo no pipeline.

tui — camada de apresentação (pode vir antes, mas o ideal é ter dados reais fluindo).

main.c — loop de simulação com cenários.

Ótimo — e parabéns por escolher assim. Aprender C "de verdade" no primeiro projeto é duro, mas é a melhor forma de entender o que linguagens como Java/C#/Python escondem de você. E isso **casa perfeitamente** com sua palestra: o tema "Além do Try/Catch" só faz sentido quando você entende o que está _embaixo_ da abstração.

Vou responder suas perguntas na ordem, explicando **por que cada coisa existe**, e depois a gente fecha o plano. Sem código pronto — só conceito, estrutura e você digitando.

---

## 1. Como um programa C vira executável (o que você PRECISA saber antes de tudo)

C não tem "runtime" que descobre coisas por você, nem build system embutido. O fluxo é sempre este:

```
arquivo.c  ──[pré-processador]──►  arquivo.i
           ──[compilador]──────►  arquivo.o   (código-objeto, não executa)
           ──[linker]──────────►  executável  (junta todos os .o + libs)
```

- **Pré-processador**: resolve `#include`, `#define`, `#ifdef`. É _textual_ — ele literalmente cola texto. Por isso `#include "foo.h"` é como "copiar e colar o conteúdo de foo.h aqui".
- **Compilador**: traduz cada `.c` isolado em `.o`. Cada `.c` é uma **unidade de tradução independente** — o compilador não vê os outros `.c`.
- **Linker**: junta os `.o` num executável e resolve os símbolos (quem chama `rb_push` precisa encontrar a definição de `rb_push` em algum `.o`).

Daí a regra de ouro dos `.h`/`.c`:

| Arquivo       | Contém                                                   | Papel           |
| ------------- | -------------------------------------------------------- | --------------- |
| `.h` (header) | **declarações** (`struct`, `enum`, protótipos de função) | O "contrato"    |
| `.c` (source) | **definições** (corpo das funções)                       | A implementação |

Se `integrity.h` declara `uint32_t crc32(const void*, size_t);`, qualquer `.c` que inclua esse header sabe _como chamar_. Mas a **implementação** fica em `integrity.c` e é o linker que liga as duas pontas.

**Por que separar?** Porque o compilador compila cada `.c` isolado, e o linker só sabe o que existe pelos símbolos. Isso te dá:

- **Manutenibilidade**: mudar o corpo de uma função não recompila o projeto todo.
- **Encapsulamento**: header expõe só o necessário.
- **Escalabilidade**: paraleliza compilação (`make -j`).

Existem também os **include guards** (ou `#pragma once`). Como `#include` é cópia literal, sem guarda um mesmo header incluído duas vezes gera declarações duplicadas e erro de compilação. A guarda é o padrão:

```c
#ifndef TELEMETRY_H
#define TELEMETRY_H
/* ...conteúdo... */
#endif
```

Você vai escrever isso em **todo** header. Pesquise "include guard" e "pragma once" para entender as duas variantes.

---

## 2. Pastas, includes, Makefile: **você cria, sim**

Nada vem "automático". O compilador não procura arquivos por convenção (como o Gradle/Maven em Java). Você decide a estrutura e você diz ao compilador onde procurar. Minha sugestão de árvore:

```
mission-critical-engine/
├── Makefile
├── README.md
├── .gitignore              ← ignora build/ e objetos
├── include/                ← headers (contratos públicos)
├── src/                    ← implementações
├── docs/                   ← arquitetura, diagramas, slides
│   └── ARCHITECTURE.md
└── build/                  ← gerado por make (.o + executável), NO GIT
```

- **`include/` vs `src/`**: separar headers de fontes é padrão em projetos C sérios. Deixa claro o que é "API pública" e o que é "implementação".
- **`build/`**: nunca commite artefatos. Vai no `.gitignore`. É o local onde o Makefile deposita `.o` e o executável.
- **`docs/`**: sua ideia é ótima — colocar arquitetura, decisões (ADRs), diagramas, esboço de slides. Isso **é** engenharia de software de verdade. Sugiro `docs/ARCHITECTURE.md` + talvez `docs/DECISIONS/ADR-001-...md` depois.

**Sobre o Makefile**: existe alternativa? Sim (`CMake`, `Meson`, `Ninja`...). Mas para um projeto C didático e pequeno, **Make é o padrão de ouro** — simples, transparente, e você aprende o que é compilação de verdade. Você vai escrever o Makefile **passo a passo**, começando mínimo (só um alvo `all` para `main.c`) e crescendo conforme adiciona módulos. **Não** escreva um Makefile gigante no dia 1.

---

## 3. "Não tem um program.c?"

**Não existe um `program.c` mágico.** Isso é confusão comum de quem vem de linguagens com "classe principal". Em C:

- Cada `.c` é uma unidade de tradução independente.
- O **executável final precisa exatamente de UMA função `main`** (o _entry point_).
- Convenção: chamar o `.c` que tem `main` de `main.c`. Mas você _pode_ chamar de qualquer coisa — o nome é escolha sua.

Então seu projeto terá vários `.c` que **não** têm `main` (`ring_buffer.c`, `integrity.c`, ...) e um único `main.c` que:

1. inclui os headers dos módulos,
2. cria os dados/estado,
3. orquestra o loop de simulação.

É exatamente assim que projetos reais funcionam: cada módulo é uma "biblioteca interna", e o `main.c` é o programa que _usa_ essas bibliotecas.

**Por que `main()` recebe `int argc, char **argv`?** Porque o sistema operacional passa os argumentos da linha de comando. Você vai usar isso para escolher o cenário da demo (`./engine --scenario corrupt`) — o que ajuda muito na palestra.

---

## 4. Idempotência (o conceito que salva seu palestrante de um desastre)

Definição formal: uma operação é **idempotente** se executá-la **N vezes** tem o **mesmo efeito final** que executá-la **uma vez**.

Exemplos do dia a dia:

- `SET contador = 5` → **idempotente** (rodar 100x termina em 5).
- `INCREMENTA contador` → **NÃO é idempotente** (100x = +100).
- `DELETAR usuário id=42` → **idempotente** (rodar 2x não quebra nada).

**Por que isso importa MUITO no seu projeto e no tema da palestra?**

Redes reais entregam mensagens em modelo **at-least-once** (pelo menos uma vez). Se seu sistema recebe um pacote, processa, e envia um ACK que se perde na rede, o emissor **vai reenviar o mesmo pacote**. Você recebe **duplicado**. Se você processar de novo sem checar:

- Uma cobrança vira duas.
- Um sensor incrementa 2x a mesma leitura.
- Um comando de "ignição" dispara duas vezes.

**Idempotência é como você garante que reprocessar é seguro.** No projeto, cada `TelemetryPacket` carrega um `seq_id` (número de sequência). O módulo `idempotency` mantém uma **janela deslizante** dos últimos N `seq_id` processados. Se chega um `seq_id` que já está na janela → **descarta como duplicado** (mas registra na TUI: "dedup"). Se chega um `seq_id` novo → **processa** e insere na janela.

Duas implementações possíveis (você escolhe depois, mas saiba que existem):

- **Bitmap circular** (`bool seen[N]` indexado por `seq_id % N`) — O(1) lookup, memória fixa. **Recomendo.**
- **Set/hash** — mais flexível, mas memória cresce. Contra o princípio "mission-critical".

A janela deslizante é o ponto onde o **ring buffer** e a **idempotência** se conectam conceitualmente — dois "buffers circulares" com propósitos diferentes. Isso rende **slide** na sua palestra.

---

## 5. Como tudo se amarra ao tema "Além do Try/Catch"

Essa é a parte que você deve destacar no README e nos slides:

| Abordagem "try/catch"                                      | Abordagem "engenharia de resiliência" (seu projeto)    |
| ---------------------------------------------------------- | ------------------------------------------------------ |
| Reativo: pega exceção _depois_ que o problema aconteceu    | Preventivo: projeta o sistema para **absorver** falhas |
| Local: captura no ponto da chamada                         | Sistêmico: isolamento, deadline, fallback, dedup       |
| Não trata falha de **rede** (duplicata, atraso, corrupção) | Trata **de verdade**: CRC, idempotência, timeout       |
| Esconde o estado, dificulta observabilidade                | Explicita métricas (TUI mostra o que aconteceu)        |

Seus quatro pilares no projeto:

1. **Ring Buffer** → _isolamento_ (produtor não bloqueia consumidor; falhas ficam confinadas).
2. **Integridade + Idempotência** → _defesa em profundidade_ (nada corrompido ou duplicado passa).
3. **Timeout + Fallback** → _degradação graciosa_ (o sistema não cai, ele muda de modo).
4. **Fault Injection** → _observabilidade/validação_ (você não confia que funciona; você **prova** com caos).

---

## 6. Ordem revisada (aprendizado incremental, cada etapa compila)

Não crie todos os arquivos vazios de uma vez. Crie **na ordem, compilando a cada passo** — porque cada passo você _entende_, e quebrou? sabe exatamente onde. Isso é o YAGNI da engenharia.

1. **Estrutura de pastas + `.gitignore` + Makefile mínimo** (só compila um `main.c` que imprime "ok"). ← seu próximo passo concreto
2. `include/telemetry.h` + `include/config.h` — o contrato de dados.
3. `integrity` — CRC32 (autocontido, fácil de testar).
4. `ring_buffer` — o coração.
5. `idempotency` — janela de `seq_id`.
6. `fault_injector` — PRNG + corrupção controlada.
7. `timeout` — deadlines com `clock_gettime`.
8. `engine` — pipeline (junta tudo).
9. `tui` — ANSI + métricas.
10. `main.c` — orquestração + cenários (`--scenario`).
11. **README + `docs/ARCHITECTURE.md` + slides** — por último, quando já existe o que documentar.

O README "beeem bonito" a gente constrói **ao final** com base no que foi feito — não prometa no README o que não existe. E isso é _outro_ conselho de engenharia.

---

## 7. Decisões que preciso de você (curtas)

Antes de começarmos o **passo 1**, responda:

1. **Threading**: sugiro **single-thread lógico** (produtor/consumidor chamados no mesmo loop, sem `pthread`). Foco no conceito sem ruído de sincronização. Ok? Alternativa é `pthread` real (mais impacto visual, mais complexidade). **Recomendo single-thread para o primeiro projeto C.**
2. **Struct do pacote**: o esqueleto do passo anterior (`seq_id`, `sensor_id`, `timestamp_ms`, `value`, `crc32`) te serve? Podemos **adicionar** campos depois — só precisamos de um primeiro contrato.
3. **Makefile mínimo do passo 1**: aceita que ele só compila `main.c` imprimindo "ok"? A gente **evolui** o Makefile junto com os módulos. Assim você aprende cada linha em vez de copiar um pronto.

Confirmando isso, eu te entrego no próximo turno:

- Um **mini-roteiro de criação de pastas/arquivos** (o que fazer no terminal, explicado),
- A **estrutura lógica do `telemetry.h` e `config.h`** em português (o que declara e por quê, sem código pronto),
- Um **Makefile mínimo comentado em prosa** (o que cada linha deveria fazer — você digita),
- E os conceitos de **ANSI escape** e `_Static_assert` que vamos usar mais à frente.

👉 Escolha: Single-thread (com simulador de concorrência no loop)

Por quê? No palco, você não quer correr risco de race condition, deadlock ou comportamento não-determinístico bem na hora da demonstração ao vivo.

Como vender isso na palestra: O single-thread com ring buffer e passos lógicos (produtor/consumidor no mesmo loop) reflete exatamente como muitos sistemas embarcados de tempo real (bare-metal ou RTOS sem concorrência pesada) funcionam. Além disso, mantém o foco do público no conceito da arquitetura (isolamento de memória) e não em detalhes de baixo nível da biblioteca pthread.

Política de Fallback ao detectar falha?
👉 Escolha: Opção (c) — Descartar, enviar para Dead-Letter Queue (DLQ) e atualizar contadores na TUI

3. Cores ANSI & Visual Hacker/Espacial: Vai ser só terminal?
   👉 Escolha: Terminal com 256 cores (ou TrueColor RGB) + TUI estruturada

meu primeiro Makefile: src/output/main.exe: src/main.c
gcc src/main.c -o src/output/main.exe

run: src/output/main.exe
.\src\output\main.exe

clean:
del /f /q src\output\main.exe

a) Include guards — o #ifndef obrigatório
Todo header precisa começar e terminar assim:

c
#ifndef NOME_H
#define NOME_H

/_ conteúdo _/

#endif /_ NOME_H _/
Por quê? #include é cópia literal de texto. Se config.h é incluído em dois .c que eventualmente se cruzam, o mesmo conteúdo entra duas vezes, e você leva erro de “redefinition”. O guard é o cinto de segurança.

Convenção de nome: CONFIG_H, TELEMETRY_H (ou MISSION_CRITICAL_ENGINE_CONFIG_H se quiser namespace — projeto solo, use o simples).

Pesquise: “include guard” vs #pragma once (a alternativa moderna, suportada pelo GCC). Para projeto C11 puro, use include guard — é o padrão portável.

(b) Tipos de largura fixa — uint32_t, não unsigned int
Para mission-critical, você nunca usa unsigned int ou long como tipo binário. Por quê? Porque o tamanho varia entre plataformas (32 vs 64 bits, Windows vs Linux). Se o protocolo de telemetria diz “seq_id tem 32 bits”, use exatamente uint32_t.

c
#include <stdint.h>
Regras do jogo:

uint8_t — 1 byte

uint16_t, int16_t — 2 bytes

uint32_t, int32_t — 4 bytes

uint64_t, int64_t — 8 bytes

Use u para “unsigned” — id, timestamp, CRC não são negativos, então u.

Pesquise: stdint.h, “integer types portable C”, “C11 fixed-width integers”.

(c) \_Static_assert — verificação em tempo de compilação
Sabe quando você quer garantir “esse struct tem exatamente 20 bytes” e falhar na compilação se alguém mudar? Isso é \_Static_assert:

c
\_Static_assert(sizeof(TelemetryPacket) == 20, "TelemetryPacket must be 20 bytes");
Por quê? Porque verificação de contrato em C precisa ser em compile time, não em runtime. Em runtime, você já pagou o preço. Compile-time é de graça. É a mesma filosofia de “falhar rápido, falhar barato” — e isso vira slide.

Cuidado: \_Static_assert só funciona dentro de struct, enum, ou no escopo de arquivo (fora de função). Se você colocar dentro de main(), o GCC 6 aceita, mas o ideal é fora. E, no seu GCC 6.3.0, \_Static_assert existe. ✅

(d) enum — status, tipos, categorias
Enums em C são números inteiros com nome. Duas coisas para saber:

Por que usar: legibilidade + verificação. PKT_OK é mais expressivo que 0. E -Wpedantic avisa se você atribuir tipo errado.

Como declarar (você digita):

Para status de pacote (retorno de processamento): PKT_OK, PKT_CORRUPTED, PKT_DUPLICATE, PKT_TIMEOUT, PKT_INVALID.

Nunca use enum como binário (persistir em disco). Enum é para uso interno — o tamanho é escolha do compilador. Para protocolo, use uint8_t com #define ou enum-como-documentação.

Pesquise: “enum in C”, “enum vs #define”, “C enum size”.

(e) struct — layout e padding
Vamos criar o TelemetryPacket. Conceitos:

Campos: seq_id (uint32_t), sensor_id (uint32_t), timestamp_ms (uint32_t), value (int32_t), crc32 (uint32_t).

Tamanho: com 5 campos uint32_t, o struct tem 20 bytes — sem padding (todo campo alinhado em 4 bytes). Perfeito.

Packing: se quiséssemos um pacote de rede exato, usaríamos **attribute**((packed)). Não precisamos aqui — 20 bytes já é o layout natural. Se tivéssemos uint8_t + uint32_t, o compilador adicionaria 3 bytes de padding, e aí precisaríamos pensar em packing. Anote isso — vira slide sobre “layout de memória”.

typedef: vamos usar typedef struct { ... } TelemetryPacket; — assim você escreve TelemetryPacket pkt; em vez de struct TelemetryPacket pkt;. Idiomático em C moderno.

Pesquise: “struct padding C”, “struct alignment C”, “**attribute**((packed))”.

(f) #define vs const vs enum para constantes
Para config.h, você tem três formas de definir constantes, e a escolha importa:

Forma Exemplo Quando usar
#define #define RING_BUFFER_CAPACITY 64 Constantes usadas em arrays (TelemetryPacket buf[RING_BUFFER_CAPACITY]) — #define é a única que funciona em tamanho de array em C11 puro
const const int x = 64; Quando você quer tipo e debugger vê o valor
enum enum { CAP = 64 }; Constantes inteiras pequenas, com escopo
Para config.h, use #define — porque vamos usá-las em tamanho de array (TelemetryPacket ring[RING_BUFFER_CAPACITY]). Em C11, const int não serv
