/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pedrferr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 17:54:31 by pedrferr          #+#    #+#             */
/*   Updated: 2026/02/25 23:15:31 by pedrferr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

int	main(int argc, char *argv[])
{
	t_rules		rules;
	t_table 	table;
	pthread_t	monitor_thread;
	int			i;
	
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
	printf("Success init!\n");
	table.start_ms = get_time_in_ms();
	
	// 1. create todas as threads
	i = 0;
	while (i < rules.num_philosophers)
	{
		// 1.1 set da última refeição para o início do processo
		pthread_mutex_lock(&table.philos[i].meal_lock);
		table.philos[i].last_meal = table.start_ms;
		pthread_mutex_unlock(&table.philos[i].meal_lock);
		
		// 1.2 inicia a thread do philo
		pthread_create(&table.philos[i].thread_id, NULL, routine, &table.philos[i]);
		i++;
	}
	
	// 1.3 inicia a thread do monitor
	pthread_create(&monitor_thread, NULL, monitor, &table);
	
	// 2. join nas threads
	i = 0;
	while (i < rules.num_philosophers)
	{
		// 2.1 join cada thread de philos
		pthread_join(table.philos[i].thread_id, NULL);
		i++;
	}
	// 2.2 join thread do monitor
	pthread_join(monitor_thread, NULL);
	
	// cleanup_table(&table); // TODO: Implement proper cleanup later
	return (1);
}
