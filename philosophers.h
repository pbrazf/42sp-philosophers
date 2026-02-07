#ifndef PHILOSOPHERS_H
#define PHILOSOPHERS_H

typedef struct s_rules {
	int num_philosophers;
	int time_to_die;
	int time_to_eat;
	int time_to_sleep;
	int num_times_each_philosopher_must_eat;
} t_rules;

#include <limits.h>

int validate_args(int argc, char *argv[]);

#endif