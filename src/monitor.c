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

void	*monitor(void *arg)
{
	t_table		*table;
	t_philo		*philos;
	int			i;
	int			num_philosophers;
	long		passed_time;

	table = (t_table *)arg;
	philos = table->philos;
	num_philosophers = table->rules.num_philosophers;
	while (1 == 1)
	{
		i = 0;
		while (i < num_philosophers)
		{
			pthread_mutex_lock(&philos[i].meal_lock);
			passed_time = get_time_in_ms() - philos[i].last_meal;
			pthread_mutex_unlock(&philos[i].meal_lock);
			if (passed_time >= table->rules.time_to_die)
			{
				pthread_mutex_lock(&table->state_mutex);
				table->stop = true;
				pthread_mutex_unlock(&table->state_mutex);
				print_status(&philos[i], "died");
				return (NULL);
			}
			i++;
			usleep(100);
		}

	}
}
