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

void	cleanup_table(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->rules.num_philosophers)
		pthread_mutex_destroy(&table->forks[i++]);
	free(table->forks);
	i = 0;
	while (i < table->rules.num_philosophers)
		pthread_mutex_destroy(&table->philos[i++].meal_lock);
	free(table->philos);
	pthread_mutex_destroy(&table->print_mutex);
	pthread_mutex_destroy(&table->state_mutex);
}

static int	cleanup_on_error(t_table *table, char *msg)
{
	int	i;

	i = 0;
	if (table->forks)
	{
		while (i < table->rules.num_philosophers)
			pthread_mutex_destroy(&table->forks[i++]);
		free(table->forks);
	}
	if (table->philos)
		free(table->philos);
	printf("%s", msg);
	return (0);
}

static int	init_philos(t_table *table)
{
	int	i;
	int	num_philosophers;

	i = 0;
	num_philosophers = table->rules.num_philosophers;
	while (i < num_philosophers)
	{
		table->philos[i].id = i + 1;
		table->philos[i].meals_eaten = 0;
		table->philos[i].last_meal = 0;
		table->philos[i].left_fork_idx = i;
		table->philos[i].right_fork_idx = (i + 1) % num_philosophers;
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
	int	num_philosophers;

	num_philosophers = table->rules.num_philosophers;
	table->forks = malloc(sizeof(pthread_mutex_t) * num_philosophers);
	if (!table->forks)
		return (cleanup_on_error(table, "Error on malloc: table->forks"));
	table->philos = malloc(sizeof(t_philo) * num_philosophers);
	if (!table->philos)
		return (cleanup_on_error(table, "Error on malloc: table->philos"));
	i = 0;
	while (i < num_philosophers)
	{
		if (pthread_mutex_init(&table->forks[i], NULL) != 0)
			return (cleanup_on_error(table, "Error init mutex fork\n"));
		i++;
	}
	if (pthread_mutex_init(&table->print_mutex, NULL) != 0)
		return (cleanup_on_error(table, "Error init print mutex\n"));
	if (pthread_mutex_init(&table->state_mutex, NULL) != 0)
		return (cleanup_on_error(table, "Error init state mutex\n"));
	if (!init_philos(table))
		return (cleanup_on_error(table,
				"Error init mutex meal_lock for one of the philosophers\n"));
	return (1);
}
