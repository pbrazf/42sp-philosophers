/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pedrferr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 17:54:31 by pedrferr          #+#    #+#             */
/*   Updated: 2026/03/08 19:12:33 by pedrferr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

static void	create_threads(t_table *table, pthread_t *monitor_thread)
{
	t_rules	rules;
	int		i;

	rules = table->rules;
	i = 0;
	while (i < rules.num_philosophers)
	{
		pthread_mutex_lock(&table->philos[i].meal_lock);
		table->philos[i].last_meal = table->start_ms;
		pthread_mutex_unlock(&table->philos[i].meal_lock);
		pthread_create(&table->philos[i].thread_id, NULL,
			routine, &table->philos[i]);
		i++;
	}
	pthread_create(monitor_thread, NULL, monitor, table);
}

static void	join_threads(t_table *table, pthread_t *monitor_thread)
{
	int	i;

	i = 0;
	while (i < table->rules.num_philosophers)
	{
		pthread_join(table->philos[i].thread_id, NULL);
		i++;
	}
	pthread_join(*monitor_thread, NULL);
}

int	main(int argc, char *argv[])
{
	t_rules		rules;
	t_table		table;
	pthread_t	monitor_thread;

	if (!validate_args(argc, argv, &rules))
	{
		printf("Error args!\n");
		return (0);
	}
	table.rules = rules;
	if (!init_table(&table))
	{
		printf("Error init!\n");
		return (0);
	}
	table.stop = false;
	table.finished_count = 0;
	table.start_ms = get_time_in_ms();
	create_threads(&table, &monitor_thread);
	join_threads(&table, &monitor_thread);
	cleanup_table(&table);
	return (0);
}
