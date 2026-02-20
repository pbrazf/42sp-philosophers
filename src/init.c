/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pedrferr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 17:54:48 by pedrferr          #+#    #+#             */
/*   Updated: 2026/02/17 17:54:48 by pedrferr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

static int cleanup_on_error(t_table *table, char *msg)
{
	int i;

	i = 0;
	if (table->forks)
	{
		while (i < table->rules.num_philosophers)
			pthread_mutex_destroy(&table->forks[i++]);
		free(table->forks);
	}
	if (table->philos)
		free(table->philos);
	ft_print(msg);
	return (0);
}

int	init_philos(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->rules.num_philosophers)
	{
		table->philos[i].id = i + 1;
		table->philos[i].meals_eaten = 0;
		table->philos[i].last_meal = 0;
		table->philos[i].left_fork_idx = i;
		table->philos[i].right_fork_idx = (i + 1) % table->rules.num_philosophers;
		table->philos[i].table = table;
		if (pthread_mutex_init(&table->philos[i].meal_lock, NULL) != 0)
			return (0);
		i++;
	}
	return (1);
}

int	init_table(t_table *table)
{
	int	i;

	table->forks = malloc(sizeof(pthread_mutex_t) * table->rules.num_philosophers);
	if (!table->forks)
		return (cleanup_on_error(table, "Error on malloc: table->forks"));
	table->philos = malloc(sizeof(t_philo) * table->rules.num_philosophers);
	if (!table->philos)
		return (cleanup_on_error(table, "Error on malloc: table->philos"));
	// 1. Iniciar Mutexes dos Garfos e Globais
	i = 0;
	while (i < table->rules.num_philosophers)
	{
		if (pthread_mutex_init(&table->forks[i], NULL) != 0)
			return (cleanup_on_error(table, "Error init mutex fork\n"));
		i++;
	}
	if (pthread_mutex_init(&table->print_mutex, NULL) != 0)
		return (cleanup_on_error(table, "Error init print mutex\n"));
	if (pthread_mutex_init(&table->state_mutex, NULL) != 0)
		return (cleanup_on_error(table, "Error init state mutex\n"));
	// 2. Iniciar Filósofos
	if (!init_philos(table))
		return (cleanup_on_error(table, "Error init mutex meal_lock for one of the philosophers\n"));
	return (1);
}
