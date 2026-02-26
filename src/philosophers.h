/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosophers.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pedrferr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 17:54:51 by pedrferr          #+#    #+#             */
/*   Updated: 2026/02/17 17:54:51 by pedrferr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILOSOPHERS_H
#define PHILOSOPHERS_H

#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct s_table t_table;
typedef struct s_philo {
	pthread_t       thread_id;      // Para dar join depois
	int             id;
	int             meals_eaten;
	long            last_meal;      // Precisa de proteção de leitura/escrita
	int             left_fork_idx;  // IDs dos garfos na mesa
	int             right_fork_idx;
	pthread_mutex_t meal_lock;      // Protege last_meal e meals_eaten DESTE filósofo
	t_table         *table;         // Ponteiro para acessar as regras/garfos
} t_philo;

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
	t_philo *philos;
	pthread_mutex_t *forks;
	pthread_mutex_t print_mutex;
	pthread_mutex_t state_mutex;
} t_table;

// parsing.c
int 	validate_args(int argc, char *argv[], t_rules *rules);

// time.c
long	get_time_in_ms(void);
void	smart_sleep(long time_in_ms, t_table *table);

// init.c
int		init_table(t_table *table);

// print.c
void	print_status(t_philo *philo, char *status);

// routine.c 
bool	check_stop(t_table *table);
void	*routine(void *arg);

// monitor.c
void	*monitor(void *arg);

#endif
