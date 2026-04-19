# Philosophers — Roadmap de Estudo e Defesa

Documento pessoal para **relembrar, revisar e defender** este projeto com
segurança. Organizado do conceito até as pegadinhas da defesa.

> Princípio-guia: você entende o código se consegue **explicar cada decisão
> de design e o que aconteceria se ela não existisse.**

---

## Índice

0. [Como usar este roadmap](#0-como-usar-este-roadmap)
1. [O problema (contexto)](#1-o-problema-contexto)
2. [Modelo mental deste projeto](#2-modelo-mental-deste-projeto)
3. [Fundamentos de concorrência que você PRECISA dominar](#3-fundamentos-de-concorrência-que-você-precisa-dominar)
4. [As decisões de design — uma por uma](#4-as-decisões-de-design--uma-por-uma)
5. [Mapa de locks (anti-deadlock)](#5-mapa-de-locks-anti-deadlock)
6. [Walkthrough por arquivo](#6-walkthrough-por-arquivo)
7. [Banco de perguntas de defesa](#7-banco-de-perguntas-de-defesa)
8. [Ferramentas de validação](#8-ferramentas-de-validação)
9. [Plano de revisão em 60 minutos](#9-plano-de-revisão-em-60-minutos)
10. [Glossário rápido](#10-glossário-rápido)

---

## 0. Como usar este roadmap

- **Primeira passada (30–60 min):** leia as seções 1–5 em ordem, só para
  recarregar o contexto. Ainda não abra o código.
- **Segunda passada (60–90 min):** abra cada arquivo enquanto lê a seção 6
  correspondente. Pare em cada "Perguntas para si mesmo" e responda em voz
  alta.
- **Terceira passada (30 min):** feche o código e responda as perguntas da
  seção 7 só com a memória. Onde travar, volte ao arquivo certo.
- **Dia da defesa:** use a seção 9 (revisão de 60 min).

---

## 1. O problema (contexto)

### 1.1 Enunciado clássico

Proposto por Dijkstra (1965). N filósofos em volta de uma mesa redonda.
Entre cada par de filósofos há **1 garfo**. Para comer, cada filósofo
precisa de **2 garfos** (o da esquerda e o da direita). Quando não come,
pensa ou dorme.

**Conflito:** se todos pegam o garfo da esquerda ao mesmo tempo, ninguém
consegue o da direita → **deadlock**.

### 1.2 Por que esse problema é clássico

É o exemplo mínimo onde aparecem **todos os grandes perigos da
concorrência**:

- **Deadlock** — ciclo de espera circular
- **Starvation** — filósofo nunca consegue comer
- **Race condition** — duas threads alterando o mesmo estado
- **Lock ordering** — sem ordem total, sem solução segura

### 1.3 Versão do subject (42) vs. versão clássica

| Requisito | Clássico | Subject |
|-----------|----------|---------|
| Estados | pensar, comer | pensar, comer, dormir |
| Critério de fim | nenhum (eterno) | opcional `must_eat`, ou primeira morte |
| Morte | ignorada | precisa anunciar em ≤10ms |
| Log | livre | `<ms> <id> <estado>` |

### 1.4 Regras de ouro do subject

1. Cada filósofo é uma **thread**.
2. Cada garfo é um **mutex**.
3. **Sem variáveis globais.**
4. Um filósofo morre se passar `time_to_die` ms desde a última refeição
   sem começar outra.
5. A mensagem de morte tem que sair em no máximo **10ms** após ela.
6. Nada pode ter **data race**.

---

## 2. Modelo mental deste projeto

### 2.1 Atores

- **N threads de filósofo** (`routine`)
- **1 thread monitor** (`monitor`)
- **N mutexes de garfo** (`table->forks[i]`)
- **1 mutex por filósofo** (`philo->meal_lock`) — protege `last_meal` e
  `meals_eaten`
- **1 mutex global de print** (`table->print_mutex`) — evita linhas
  intercaladas
- **1 mutex global de estado** (`table->state_mutex`) — protege `stop`

### 2.2 Estado (structs)

```
t_rules
├── num_philosophers
├── time_to_die
├── time_to_eat
├── time_to_sleep
└── num_times_each_philosopher_must_eat  (-1 = infinito)

t_philo (um por filósofo)
├── thread_id
├── id (1..N)
├── meals_eaten
├── last_meal          ← protegido por meal_lock
├── left_fork_idx  = i
├── right_fork_idx = (i+1) % N
├── meal_lock
└── table*  (volta para t_table)

t_table (estado global)
├── rules
├── start_ms           (referência de tempo absoluta do início)
├── stop               ← protegido por state_mutex
├── finished_count
├── philos[N]
├── forks[N]           (array de mutexes)
├── print_mutex
└── state_mutex
```

### 2.3 Linha do tempo de uma execução

```
main()
  validate_args         → erro ⇒ "Error args!" e sai 0
  init_table            → erro ⇒ "Error init!" e sai 0
  stop = false
  start_ms = agora      ← t=0 do log
  create_threads
    para cada filo:
      lock meal_lock; last_meal = start_ms; unlock   ← crítico
      pthread_create(routine)
    pthread_create(monitor)
  join_threads          (espera todos)
  cleanup_table         (destroi mutexes, libera memória)
  return 0
```

---

## 3. Fundamentos de concorrência que você PRECISA dominar

### 3.1 Thread vs. processo

- **Processo:** espaço de memória isolado. Criação cara.
- **Thread:** compartilha memória (heap, globais) com outras threads do
  mesmo processo. Criação barata. Comunicação fácil, mas **perigosa**
  (precisa sync).
- Usamos threads aqui porque o monitor precisa LER o estado dos
  filósofos. Se fossem processos, precisaríamos de semáforos, memória
  compartilhada ou IPC — que é o caminho do **bonus** (`philo_bonus/`).

### 3.2 Mutex (mutual exclusion)

- Primitivo de sincronização: só 1 thread pode estar "dentro" do lock.
- `pthread_mutex_init` → cria. `destroy` → libera. `lock` bloqueia
  enquanto outra thread segura. `unlock` libera.
- **Seção crítica:** trecho de código entre lock e unlock. Queremos ela
  **curta** (contenção) e **suficiente** (proteger todos acessos ao
  estado compartilhado).

### 3.3 Race condition

Quando duas threads acessam a mesma variável e ao menos uma escreve,
**sem sincronização**, o resultado é indefinido (UB em C).

**Exemplo neste projeto:** sem `meal_lock`, o monitor poderia ler
`last_meal` enquanto a thread do filósofo o atualiza → leitura
"quebrada" (parte nova, parte antiga).

### 3.4 Deadlock (4 condições de Coffman)

Acontece se e somente se **TODAS** as 4 ocorrem:

1. **Exclusão mútua** — recurso só pode ser usado por 1 thread
2. **Hold & wait** — thread segura um e pede outro
3. **No preemption** — não dá pra tirar à força
4. **Circular wait** — A espera B, B espera A

Quebrar **UMA** dessas 4 = sem deadlock. Nós quebramos **circular
wait** impondo ordem total nos garfos (sempre pega o de **menor
índice** primeiro).

### 3.5 Starvation

Thread não trava, mas **nunca consegue** o recurso. O DP clássico é
propenso: se o escalonador favorece alguns, outros nunca comem. Em
nossa solução, o atraso par/ímpar + `must_eat` ajudam a evitar, mas não
é uma garantia formal (nem o subject exige).

---

## 4. As decisões de design — uma por uma

### 4.1 Por que um monitor separado?

Um filósofo pode estar **dormindo** ou **bloqueado esperando garfo**. Se
ele próprio precisasse verificar a morte, poderia não conseguir a tempo.
Um monitor dedicado faz poll centralizado.

**Alternativa ruim:** condicional após cada ação — inflacionaria código
e teria a mesma janela de atraso em blocos longos (`time_to_eat`).

### 4.2 Por que ordem total de garfos (menor índice primeiro)?

Quebra a condição #4 de Coffman (circular wait). Se todas as threads
obedecem a **mesma ordem global** ao pedir locks, nenhum ciclo existe.

**Alternativa clássica:** um garfo por vez com trylock — funciona, mas
inflaciona contexto. Ordenar índices é simples e provavelmente correto.

### 4.3 Atraso par/ímpar no início

```c
if (philo->id % 2 == 0)
    usleep(rules->time_to_eat * 500);   // 0.5 * time_to_eat em μs
```

Se todos os filósofos começam ao mesmo tempo e vão direto pro garfo,
contenção máxima. Atrasar os pares "desintercala" as requisições e
reduz o tempo médio de espera.

**Fator 500** (não 1000): metade de `time_to_eat` em microssegundos.
Empiricamente funciona bem.

### 4.4 `smart_sleep` em vez de `usleep` direto

`usleep(time_to_sleep * 1000)` tem 2 problemas:
1. Não reage a `stop` → thread continua dormindo depois que o monitor
   decretou o fim da simulação.
2. Precisão ruim em valores grandes.

`smart_sleep` faz loop com `usleep(100)` (100μs) checando stop, até
atingir o tempo alvo.

### 4.5 Granularidade de 100μs no monitor

Subject exige que a morte seja anunciada em ≤10ms. O monitor polla a
cada 100μs = 0,1ms. Orçamento folgado, mas sem consumir CPU demais.

### 4.6 Atualizar `last_meal` **antes** de `print "is eating"`

```c
lock(meal_lock);
philo->last_meal = agora;   ← primeiro atualiza
philo->meals_eaten++;
unlock(meal_lock);
print_status("is eating");  ← depois imprime
```

**Por quê?** Se o print estivesse antes, o monitor poderia entrar entre
o print e o update, ler um `last_meal` antigo e decretar morte falsa.
Com o update primeiro, o "relógio de fome" é **zerado antes** da
janela de visibilidade.

### 4.7 Caso de 1 filósofo

Com 1 filo e 1 garfo, **comer é impossível** (precisaria de 2 garfos).
Então:

```c
pega garfo esquerdo
print "has taken a fork"
smart_sleep(time_to_die)
libera garfo
```

O monitor mata o filósofo exatamente no tempo certo.

### 4.8 Sem variáveis globais

Subject proíbe. Tudo fica em `t_table` e é passado por ponteiro para as
threads. Disciplina que paga em portabilidade e testabilidade.

### 4.9 "died" precisa imprimir mesmo com `stop == true`

```c
if (!check_stop(table) || !ft_strcmp(status, "died"))
    printf(...);
```

O monitor seta `stop = true` **antes** de chamar `print_status("died")`
(para parar os outros filósofos). Se a regra fosse só "não imprime se
stop", a morte nunca seria anunciada. Exceção explícita resolve.

### 4.10 Retorno 0 mesmo em erro de args/init

```c
printf("Error args!\n");
return (0);
```

O subject pede que o programa imprima erro e saia com status 0 (comum
em projetos 42, evita tratamento especial no avaliador).

---

## 5. Mapa de locks (anti-deadlock)

Ordens de aquisição por função:

| Função            | Locks pegos (em ordem)                            |
|-------------------|---------------------------------------------------|
| `take_forks`      | `forks[min(left, right)]` → `forks[max(...)]`     |
| `eat`             | `meal_lock` (isolado)                             |
| `print_status`    | `print_mutex` → `state_mutex` (via `check_stop`)  |
| `check_stop`      | `state_mutex` (isolado)                           |
| `check_meals`     | `meal_lock` (isolado, por filósofo)               |
| `check_life`      | `state_mutex` (set stop) → depois `print_mutex` → `state_mutex` via `print_status` |

**Propriedade:** qualquer par de funções tem uma ordem **compatível**
(não há ciclo). Prova curta:
- Garfos só aparecem em `take_forks`, e só em ordem crescente de índice.
- `meal_lock` nunca é adquirido junto com nenhum outro.
- `print_mutex` sempre vem antes de `state_mutex`.

Não há ciclo no grafo de precedência → sem deadlock.

---

## 6. Walkthrough por arquivo

Para cada arquivo: **o que faz**, **pontos críticos**, **perguntas para si
mesmo**. Tudo que você quiser checar, abra o arquivo em paralelo.

### 6.1 [philosophers.h](../philo/philosophers.h)

**O que contém:**
- Includes do sistema.
- Forward declaration: `typedef struct s_table t_table;` (porque
  `t_philo` tem `t_table *` e vice-versa).
- Structs `t_philo`, `t_rules`, `t_table`.
- Protótipos, agrupados por arquivo.

**Perguntas para si mesmo:**
- Por que `last_meal` é `long` e não `int`? (ms desde Epoch cabe só em
  long — overflow de int ~24 dias).
- Por que `meal_lock` é por filósofo e não um só global? (granularidade
  fina = menos contenção; monitor lê um sem travar todos).

### 6.2 [main.c](../philo/main.c)

**Fluxo:** validate → init → `stop=false` → `start_ms=agora` →
create_threads → join → cleanup → return 0.

**Detalhe crítico — `create_threads`:**

```c
lock(meal_lock);
philo->last_meal = start_ms;
unlock(meal_lock);
pthread_create(..., routine, philo);
```

Por que setar `last_meal = start_ms` **antes** de criar a thread?
Porque o monitor pode começar a rodar imediatamente. Se `last_meal`
começasse em 0, o monitor veria `passed_time = agora - 0 = gigante` e
decretaria morte imediata.

**Perguntas para si mesmo:**
- E se um `pthread_create` falhar? (hoje não tratamos; melhoria
  possível — mas ninguém vai pedir pra você consertar isso).

### 6.3 [parsing.c](../philo/parsing.c)

**O que faz:** valida `argc ∈ {5, 6}` e cada argv como inteiro positivo.

**`parse_int_pos` rejeita:**
- `NULL` ou string vazia
- Qualquer caractere não-dígito (logo `-5` cai no `-`)
- Zero
- Overflow de `long`
- Valor `> INT_MAX`

**Sentinela `-1`** em `num_times_each_philosopher_must_eat` quando
`argc == 5` → monitor nunca encerra por critério de refeição.

**Perguntas para si mesmo:**
- `./philo 5 800 200 200 0` — aceita? Não (zero rejeitado por
  `parse_int_pos`).
- E `+5`? Não, `+` não é dígito.
- E espaços antes/depois? Não, `parse_int_pos` não ignora espaços.

### 6.4 [init.c](../philo/init.c)

**`init_table`:**
1. Aloca `forks[N]`
2. Aloca `philos[N]`
3. Inicializa cada `forks[i]` (mutex)
4. Inicializa `print_mutex`, `state_mutex`
5. Inicializa cada `philo->meal_lock` + campos (id, idx dos garfos, ptr
   pra table)

**`cleanup_on_error`:** em falha no meio, libera o que já foi alocado e
printa mensagem. Retorna 0 para o caller.

**`cleanup_table`:** destrói todos os mutexes e libera memória.

**Perguntas para si mesmo:**
- Se falha a inicialização do mutex `forks[2]` de 5, os 3 já
  inicializados vazam? (Não — `cleanup_on_error` destrói até `N-1`,
  mas `pthread_mutex_destroy` em mutex não inicializado é no-op na
  glibc. Aceitável.)
- Por que `left_fork_idx = i` e `right_fork_idx = (i+1) % N`?
  Circularidade da mesa.

### 6.5 [time.c](../philo/time.c)

**`get_time_in_ms`:** `gettimeofday` → segundos × 1000 + μs / 1000.
Retorna ms desde Epoch.

**`smart_sleep(tempo_ms, table)`:** loop com `usleep(100)` checando
`check_stop` até atingir o alvo.

**Perguntas para si mesmo:**
- Por que não `clock_gettime`? Subject só permite `gettimeofday`.
- E se o sistema tiver mudança de horário durante a simulação?
  Subject ignora isso na prática.

### 6.6 [print.c](../philo/print.c)

**`print_status`:**
1. Lock `print_mutex` (linhas não intercalam).
2. Se `!stop` **ou** status == "died" → imprime.
3. Unlock.

`ft_strcmp` é local, usado só para detectar "died".

**Perguntas para si mesmo:**
- Por que ordem print → state_mutex? Para evitar deadlock com outras
  funções que fazem state-first (não há — mas mantém a convenção).

### 6.7 [routine.c](../philo/routine.c)

**`check_stop`:** lê `stop` sob `state_mutex`. Chamado o tempo todo.

**`take_forks`:** ordena por índice, pega menor → print → pega maior →
print. **Ordem total** → sem deadlock.

**`eat`:**
1. Lock `meal_lock`
2. `last_meal = agora`, `meals_eaten++`
3. Unlock
4. `print_status("is eating")`
5. `smart_sleep(time_to_eat)`

**`alone_philo_case`:** 1 garfo, `smart_sleep(time_to_die)`, libera.

**Loop principal:**
```
se id par: usleep(time_to_eat * 500)   // defasa
enquanto !stop:
  se N == 1: alone_case; return
  take_forks
  eat
  libera os dois garfos
  print "is sleeping"; smart_sleep(time_to_sleep)
  print "is thinking"
  se N ímpar: usleep(500)   // ajuda escalonamento
```

**Perguntas para si mesmo:**
- Por que libera garfo esquerdo antes do direito? Não importa —
  qualquer ordem de **unlock** é segura.
- Por que o `usleep(500)` só quando N é ímpar? Com N ímpar não dá para
  alternar par/ímpar perfeitamente; esse empurrão ajuda a suavizar.

### 6.8 [monitor.c](../philo/monitor.c)

**`check_life`:** se `passed_time >= time_to_die` → set `stop = true`
(sob state_mutex) → `print_status("died")`.

**`check_meals`:** para cada filósofo:
- Lock `meal_lock` → lê `passed_time = agora - last_meal` e checa
  `meals_eaten` → unlock.
- Chama `check_life` **depois** de liberar `meal_lock` (importante — se
  `check_life` chamasse print e print tentasse algum lock inesperado,
  não haveria conflito).

**Loop principal:**
- A cada iteração, varre todos os filósofos.
- Se `must_eat` != -1 e todos comeram o suficiente → set stop, encerra.
- `usleep(100)`.

**Perguntas para si mesmo:**
- Por que `finished = 1` no começo do loop externo e é colocado em 0
  dentro? Assume que todos terminaram; qualquer um que não comeu o
  bastante derruba a flag.
- Por que `check_life` é chamado para **cada** filósofo dentro de
  `check_meals`? Para que a morte seja detectada imediatamente, mesmo
  no primeiro da varredura.

---

## 7. Banco de perguntas de defesa

### Essenciais (sabe ou não passa)

1. **Explique o problema dos filósofos jantando.**
2. **O que é deadlock? Como você garante que não ocorre no seu
   código?** → Ordem total de aquisição de garfos (menor índice
   primeiro).
3. **O que é race condition? Dê um exemplo concreto no seu código e
   mostre o mutex que a evita.** → `last_meal` / `meal_lock`; `stop` /
   `state_mutex`.
4. **Por que você tem uma thread monitor?** → Filósofos podem estar
   bloqueados ou dormindo e perderiam a janela de 10ms.
5. **Por que `usleep(100)` no monitor?** → 0,1ms de granularidade;
   subject exige ≤10ms → folga.
6. **Por que atualizar `last_meal` ANTES de imprimir "is eating"?** →
   Evita que o monitor veja tempo defasado e declare morte falsa.
7. **Como funciona o caso de 1 filósofo?** → Pega 1 garfo, dorme
   `time_to_die`, o monitor mata.
8. **Por que o atraso par/ímpar?** → Reduz contenção inicial.

### Intermediárias

9. **Desenhe o grafo de aquisição de locks.** → ver seção 5.
10. **Sua solução é fair / starvation-free?** → Não provada; o atraso e
    o `must_eat` reduzem na prática.
11. **Diferença entre `pthread_join` e `pthread_detach`?** → Join
    espera e coleta retorno; detach descarta e libera recursos
    automaticamente.
12. **Por que `num_philosophers` não tem mutex?** → Só é escrito em
    init (single-threaded); todas as leituras depois disso são
    read-only.
13. **O que acontece se `time_to_die < time_to_eat`?** → Impossível
    comer antes de morrer; monitor mata no meio da refeição.
14. **Como você detecta memory leaks? E races?** → valgrind,
    helgrind, ThreadSanitizer.

### Avançadas / pegadinhas

15. **E se eu passar 0 filósofos?** → Rejeitado por `parse_int_pos`
    (zero explicitamente proibido).
16. **E overflow de int no argv?** → Rejeitado.
17. **Mutex destruído antes de unlock?** → Não acontece: `cleanup`
    roda só depois de `pthread_join` de todos.
18. **E se o `pthread_mutex_init` de um garfo no meio falhar?** →
    `cleanup_on_error` destrói o que foi criado; `pthread_mutex_destroy`
    em mutex não inicializado é no-op na glibc.
19. **Por que o monitor não pega `print_mutex` diretamente?** → Porque
    ele chama `print_status`, que já o pega. Evita locks duplos.
20. **O que é "fair mutex"? O pthread é fair?** → Não é garantido; POSIX
    não especifica a ordem de aquisição. Implementações podem variar.

---

## 8. Ferramentas de validação

| Ferramenta | Comando (dentro de `philo/`)                          | Detecta |
|------------|--------------------------------------------------------|---------|
| Compilador | `make` (CFLAGS com `-Wall -Wextra -Werror`)            | Warnings como erro |
| Norminette | `norminette *.c *.h`                                   | Estilo 42 |
| Valgrind   | `valgrind --leak-check=full --show-leak-kinds=all ./philo ...` | Leaks, UB de heap |
| Helgrind   | `valgrind --tool=helgrind ./philo ...`                 | Data races, violação de lock order |
| TSan       | Recompilar com `-fsanitize=thread -g` e rodar          | Races (com menos overhead que helgrind) |

**Cenários de teste que valem mostrar:**

```
./philo 1 800 200 200             # morre em 800ms
./philo 5 800 200 200 7           # termina limpo (must_eat)
./philo 4 410 200 200             # timing apertado, deve morrer
./philo 5 800 200 200             # "saudável", sem mortes em runs longos
./philo abc 600 200 200           # erro de args
./philo 0 800 200 200             # erro de args (zero)
```

---

## 9. Plano de revisão em 60 minutos

(Fazer no dia/noite anterior à defesa.)

| Tempo  | Atividade |
|--------|-----------|
| 0–5    | Ler `philosophers.h` — decorar structs |
| 5–15   | Desenhar no papel: fluxo de `main`, ciclo de `routine`, loop de `monitor` |
| 15–25  | Desenhar grafo de locks e citar ordem em cada função |
| 25–35  | Responder em voz alta as 8 perguntas essenciais (seção 7) |
| 35–45  | Rodar: 1 filo, `5 800 200 200 7`, `4 410 200 200` |
| 45–50  | Abrir valgrind e helgrind em 1 cenário, mostrar zero erros |
| 50–60  | Pegar 2 perguntas avançadas e treinar respondendo devagar, em voz alta — incluindo dúvida ("não tenho certeza, mas pensaria assim...") |

---

## 10. Glossário rápido

- **Thread** — unidade de execução dentro de um processo; compartilha
  memória.
- **Mutex** — lock para seção crítica.
- **Seção crítica** — trecho entre lock e unlock; só 1 thread por vez.
- **Data race** — acesso concorrente a mesma variável sem sync, com ao
  menos 1 escrita.
- **Deadlock** — threads travadas esperando umas às outras em ciclo.
- **Starvation** — thread nunca consegue recurso (sem travar o
  sistema).
- **Ordem total de locks** — convenção global sobre ordem de aquisição;
  previne circular wait.
- **Polling** — checar um estado repetidamente em loop.
- **Busy-wait** — polling sem dormir; desperdiça CPU.
- **Monitor (thread)** — neste projeto: thread dedicada a observar
  estado dos filósofos.
- **`gettimeofday`** — chamada POSIX para obter tempo atual.
- **`usleep`** — dorme por microssegundos.

---

*Última atualização: 2026-04-19. Autor: revisão pessoal baseada no
código em `philo/`.*
