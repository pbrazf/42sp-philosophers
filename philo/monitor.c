/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pedrferr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 22:01:27 by pedrferr          #+#    #+#             */
/*   Updated: 2026/02/25 22:01:27 by pedrferr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

static int	check_life(t_table *table, long passed_time, t_philo *philo)
{
	if (passed_time >= table->rules.time_to_die)
	{
		pthread_mutex_lock(&table->state_mutex);
		table->stop = true;
		pthread_mutex_unlock(&table->state_mutex);
		print_status(philo, "died");
		return (0);
	}
	return (1);
}

static int	check_meals(t_table *table, int *finished, int num_philosophers)
{
	t_philo		*philos;
	int			i;
	long		passed_time;

	i = 0;
	philos = table->philos;
	while (i < num_philosophers)
	{
		pthread_mutex_lock(&philos[i].meal_lock);
		passed_time = get_time_in_ms() - philos[i].last_meal;
		if (philos[i].meals_eaten
			< table->rules.num_times_each_philosopher_must_eat)
			*finished = 0;
		pthread_mutex_unlock(&philos[i].meal_lock);
		if (!check_life(table, passed_time, &philos[i]))
			return (0);
		i++;
	}
	return (1);
}

void	*monitor(void *arg)
{
	t_table		*table;
	int			num_philosophers;
	int			finished;

	table = (t_table *)arg;
	num_philosophers = table->rules.num_philosophers;
	while (1 == 1)
	{
		finished = 1;
		if (!check_meals(table, &finished, num_philosophers))
			return (NULL);
		if (table->rules.num_times_each_philosopher_must_eat != -1 && finished)
		{
			pthread_mutex_lock(&table->state_mutex);
			table->stop = true;
			pthread_mutex_unlock(&table->state_mutex);
			return (NULL);
		}
		usleep(100);
	}
}
