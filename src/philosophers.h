#ifndef PHILOSOPHERS_H
#define PHILOSOPHERS_H

typedef struct s_rules {
	int num_philosophers;
	int time_to_die;
	int time_to_eat;
	int time_to_sleep;
	int num_times_each_philosopher_must_eat;
} t_rules;

typedef struct s_table {
	t_rules rules;
	long start_ms;
	bool stop;
	int finished_count;
	pthread_mutex_t *forks;
	pthread_mutex_t print_mutex;
	pthread_mutex_t state_mutex;
} t_table;

#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>

int validate_args(int argc, char *argv[], t_rules *rules);

#endif