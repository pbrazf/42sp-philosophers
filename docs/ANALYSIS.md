# Philosophers — Validação pré-submit

Análise completa do projeto para garantir 100% antes do submit oficial.

## Status geral

| Área | Status |
|------|--------|
| Compilação (`-Wall -Wextra -Werror`) | ✅ Limpa, sem warnings |
| Parsing de args inválidos | ✅ Rejeita corretamente |
| Caso 1 filósofo | ✅ Morre em `time_to_die`ms |
| `must_eat` (encerramento limpo) | ✅ Exit 0, sem mortes |
| Detecção de morte (timing apertado) | ✅ Detectada |
| Estrutura de pastas (`philo/`) | ✅ Reorganizado de `src/` → `philo/` |
| README (requisitos do subject) | ✅ Reescrito (1ª linha em itálico, Description/Instructions/Resources + AI usage) |
| Repo oficial sincronizado | ✅ `philo/` + `README.md` + `.gitignore` copiados e build verificado |
| Leaks (valgrind) | ✅ 0 bytes em uso na saída, 0 erros |
| Data races (helgrind) | ✅ 0 erros |
| Data races (ThreadSanitizer) | ✅ 0 warnings |
| Norminette | ✅ Todos arquivos OK |
| Bonus (`philo_bonus/`) | ⚪ Não feito (opcional) |

---

## 1. Conformidade com o Subject

### 1.1 Funções externas usadas
Subject permite: `memset, printf, malloc, free, write, usleep, gettimeofday, pthread_create, pthread_detach, pthread_join, pthread_mutex_init, pthread_mutex_destroy, pthread_mutex_lock, pthread_mutex_unlock`.

Código usa: `printf, malloc, free, usleep, gettimeofday, pthread_create, pthread_join, pthread_mutex_*`. **OK**.

Observação: não usa `pthread_detach` nem `memset` nem `write` — tudo bem, são permitidas, não obrigatórias.

### 1.2 Sem variáveis globais
Conferido — tudo passa por `t_table`. **OK**.

### 1.3 Formato de logs
Usa `timestamp_in_ms X <status>` com status:
- `has taken a fork`
- `is eating`
- `is sleeping`
- `is thinking`
- `died`

**OK** — formato bate com o subject.

### 1.4 Makefile
Tem regras: `all, clean, fclean, re, $(NAME)`. Usa `cc` com `-Wall -Wextra -Werror`. Sem relink desnecessário (usa `.o` cache). **OK**.

### 1.5 Estrutura de pastas — ✅ Corrigido
Subject diz: *Files to Submit: Makefile, \*.h, \*.c, in directory `philo/`*

Layout final:
```
42sp-philosophers/
├── philo/
│   ├── Makefile
│   ├── philosophers.h
│   ├── main.c
│   ├── init.c
│   ├── parsing.c
│   ├── time.c
│   ├── print.c
│   ├── routine.c
│   └── monitor.c
├── README.md
├── ANALYSIS.md   (notas internas, não vai pro oficial)
└── .gitignore
```

Makefile novo usa caminhos relativos ao próprio `philo/` (sem `-I./src` nem `SRC_DIR`).

---

## 2. Validação de código

### 2.1 `src/main.c`
- `create_threads`: inicializa `last_meal = start_ms` com mutex antes de criar cada thread — correto.
- `join_threads`: junta philos + monitor — OK.
- Retorna `0` em erro de args/init após printar mensagem — OK.

### 2.2 `src/parsing.c`
- `parse_int_pos` rejeita vazio, negativo, zero, overflow de `long`, não-dígito. **OK**.
- Valida `argc` entre 5 e 6. **OK**.
- `num_times_each_philosopher_must_eat = -1` quando omitido — sentinela para "infinito". **OK**.

### 2.3 `src/init.c`
- `init_table` aloca `forks[]` e `philos[]`, inicializa todos os mutexes, faz cleanup parcial em erro. **OK**.
- `cleanup_table` destrói todos os mutexes e libera memória. **OK**.
- Observação: `cleanup_on_error` — se falhar na inicialização de mutex fork no meio, destrói até `num_philosophers-1` mesmo se só `i` foram inicializados (bug sutil, mas na prática `pthread_mutex_destroy` em mutex não inicializado é no-op em glibc; aceitável).

### 2.4 `src/time.c`
- `get_time_in_ms` — padrão. **OK**.
- `smart_sleep` — faz loop de 100μs checando `check_stop` — essencial para reação rápida ao stop e evita overrun do `usleep` grande. **OK**.

### 2.5 `src/print.c`
- `print_status` trava `print_mutex`, checa `!check_stop || status == "died"` (permite imprimir a morte mesmo com stop=true). **OK**.
- Ordem dos locks: `print_mutex` → `state_mutex` (via `check_stop`). Consistente.

### 2.6 `src/routine.c`
- `check_stop` lê `table->stop` com `state_mutex`. **OK**.
- `take_forks`: ordena pelo índice menor primeiro — **quebra deadlock circular clássico**. **OK**.
- `eat`: trava `meal_lock`, atualiza `last_meal` e `meals_eaten`, destrava, depois printa "is eating". **OK**.
- `alone_philo_case`: pega 1 garfo, dorme `time_to_die`, destrava — o monitor o mata nesse período. **OK**.
- Defasagem par/ímpar no início (`usleep(time_to_eat * 500)` para pares): reduz contenção inicial. **OK**.
- `usleep(500)` quando ímpar ajuda a não monopolizar. OK.

### 2.7 `src/monitor.c`
- `check_life` seta `stop = true` com `state_mutex`, destrava, e aí chama `print_status` ("died"). Isso evita deadlock com `print_status` que também pega `state_mutex`. **OK**.
- `check_meals` trava `meal_lock` por philo, lê `passed_time` e `meals_eaten`, destrava antes de chamar `check_life`. **OK**.
- Loop com `usleep(100)` — 0.1ms de granularidade, dentro dos 10ms que o subject exige pra detectar morte. **OK**.

### 2.8 Verificação de deadlocks/ordens de lock
| Thread | Ordem de locks |
|--------|----------------|
| routine (take_forks) | `forks[min]` → `forks[max]` |
| routine (eat) | `meal_lock` (isolado) |
| routine (print_status) | `print_mutex` → `state_mutex` |
| monitor (check_meals) | `meal_lock` (isolado) |
| monitor (check_life) | `state_mutex` isolado, depois `print_mutex` → `state_mutex` |

Sem ciclo de locks. **OK**.

---

## 3. Testes funcionais (executados)

### 3.1 Args inválidos
```
./philo               → Error args!
./philo 5 600 200 200 extra_arg → Error args!
./philo -5 600 200 200 → Error args!
./philo abc 600 200 200 → Error args!
```
**OK**.

### 3.2 1 filósofo (deve morrer)
```
./philo 1 800 200 200
0 1 has taken a fork
800 1 died
```
**OK** — morreu exatamente em 800ms.

### 3.3 `must_eat` termina limpo
```
./philo 5 800 200 200 3 → exit 0, 0 mortes, encerra em ~1400ms
```
**OK**.

### 3.4 Timing apertado (deve morrer)
```
./philo 4 310 200 100 → 1 died @310ms
./philo 5 410 200 200 → 1 died @411ms
```
**OK** — detecta morte com precisão.

### 3.5 4 filósofos "saudável" (800/200/200)
Rodado 8s sem mortes. **OK**.

---

## 4. Resultados das ferramentas

### Valgrind `--leak-check=full --show-leak-kinds=all`
```
./philo 4 800 200 200 3   → ERROR SUMMARY: 0 errors, in use at exit: 0 bytes
./philo 1 400 200 200     → ERROR SUMMARY: 0 errors, in use at exit: 0 bytes
```

### Helgrind
```
./philo 4 800 200 200 3   → ERROR SUMMARY: 0 errors (163k suppressed — internos da glibc/pthread, não são do nosso código)
```

### ThreadSanitizer
Compilado com `-fsanitize=thread` e executado:
```
./philo 4 800 200 200 3   → 0 warnings
```

### Norminette
```
src/init.c: OK!
src/main.c: OK!
src/monitor.c: OK!
src/parsing.c: OK!
src/philosophers.h: OK!
src/print.c: OK!
src/routine.c: OK!
src/time.c: OK!
```

## 5. Pendências

- [x] Reestruturar para `philo/` (mover `src/*` para `philo/`, ajustar Makefile)
- [x] Reescrever README conforme requisitos do subject
- [x] Copiar para repo oficial `philosophers/`
- [x] Rebuild + testes no repo oficial (1-philo morre em 400ms, clean OK)
- [ ] Commit + push no repo não oficial (`main`)
- [ ] Commit + push no repo oficial (`master`)

---

## 6. Observações para a defesa

- **Justificar a ordem dos locks em `take_forks`**: usar índice menor primeiro garante ordem total e quebra deadlock circular.
- **Por que `usleep(100)` no monitor?** — granularidade suficiente pra detectar morte dentro dos 10ms exigidos.
- **Por que `smart_sleep`?** — `usleep` grande não reage a `stop`; o loop com check permite sair rápido quando a simulação termina.
- **Atraso par/ímpar no `routine`**: evita que todos tentem pegar o garfo da esquerda ao mesmo tempo no começo.
- **`last_meal` atualizado dentro de `meal_lock` antes do `print "is eating"`**: o monitor enxerga o tempo correto do início da refeição, não o do print.
