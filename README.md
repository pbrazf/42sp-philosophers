*This project has been created as part of the 42 curriculum by pedrferr.*

# Philosophers

## Description

Philosophers is a 42 project that implements the classic Dining Philosophers problem in C using POSIX threads and mutexes. The goal is to simulate a group of philosophers sitting at a round table who alternate between eating, sleeping, and thinking — without ever starving to death.

Each philosopher is a thread, each fork is a mutex, and a dedicated monitor thread watches for starvation and the optional meal-count termination condition. The implementation focuses on:

- Correct, race-free synchronization between philosophers and the monitor
- Accurate millisecond-resolution timestamps
- Strict death detection and logging constraints (a death must be announced within 10 ms of the actual event)
- Graceful shutdown with no leaked memory or undestroyed mutexes

## Instructions

### Build

From the repository root:

```
cd philo
make
```

This produces a `philo` binary inside `philo/`.

Other Makefile rules: `clean`, `fclean`, `re`.

### Run

```
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

- `number_of_philosophers`: number of philosophers (and forks) at the table.
- `time_to_die` (ms): if a philosopher does not start a meal within this window since their previous meal, they die.
- `time_to_eat` (ms): how long a meal takes (two forks held).
- `time_to_sleep` (ms): how long a philosopher sleeps after eating.
- `number_of_times_each_philosopher_must_eat` (optional): if provided, the simulation stops once every philosopher has eaten at least this many times.

### Output format

Each state change is printed as:

```
<timestamp_in_ms> <philosopher_id> <state>
```

Where `<state>` is one of: `has taken a fork`, `is eating`, `is sleeping`, `is thinking`, `died`.

### Examples

```
./philo 1 800 200 200        # one philosopher dies after 800 ms
./philo 5 800 200 200 7      # five philosophers, ends after each eats 7 meals
./philo 4 410 200 200        # tight timing, tests death detection
```

## Resources

Classic references used while working on this project:

- [*The Dining Philosophers problem* (Wikipedia)](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [POSIX Threads Programming (LLNL)](https://hpc-tutorials.llnl.gov/posix/)
- `man` pages: `pthread_create`, `pthread_join`, `pthread_mutex_init`, `pthread_mutex_lock`, `gettimeofday`, `usleep`.
- *The Little Book of Semaphores*, Allen B. Downey (conceptual background on synchronization primitives).

### AI usage

AI (Claude Code) was used exclusively as a review and validation assistant after the project was already implemented. Specifically, it was used to:

- Re-read the full codebase and cross-check it against the subject (allowed functions, Norm, output format, global rules).
- Run and interpret validation tools: `make` with the required flags, Norminette, Valgrind (`--leak-check=full`), Helgrind, and ThreadSanitizer.
- Design and run a battery of functional tests (single-philosopher death case, tight-timing death detection, `must_eat` termination, long healthy runs).
- Reorganize the project layout to match the `philo/` directory structure required by the subject.
- Draft this README.

AI was **not** used to write the core project source code (thread routine, monitor, synchronization logic, parsing, time, or print). Those were authored manually; AI only reviewed them.

## Technical notes

- Each philosopher thread sorts fork indices by value before locking to enforce a global lock ordering and avoid the classic circular-wait deadlock.
- The monitor thread polls every 100 µs, comfortably inside the 10 ms death-detection budget required by the subject.
- `last_meal` is updated under a per-philosopher `meal_lock` before the "is eating" log is printed, so the monitor always sees the meal's true start time.
- A `smart_sleep` function replaces a single long `usleep`, checking the stop flag periodically so the simulation reacts promptly when it ends.
- There are no global variables; all state lives inside a single `t_table` passed to the threads.
