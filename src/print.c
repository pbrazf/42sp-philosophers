/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pedrferr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 00:44:56 by pedrferr          #+#    #+#             */
/*   Updated: 2026/02/22 00:44:56 by pedrferr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

void print_status(t_philo *philo, char *status)
{
	long	time_now;

    // tranca o print_mutex
	pthread_mutex_lock(&philo->table->print_mutex);

    // confere rapidinho se a simulacao parou (pra não imprimir coisas pós-morte)
	if (!check_stop(philo->table) || status[0] == 'd') // TODO: FAZER UM STRCMP DEPOIS
	{
    	time_now = get_time_in_ms() - philo->table->start_ms; // Precisa ser o tempo com base na hora que os filósofos sentaram na mesa
		printf("%ld %d %s\n", time_now, philo->id, status);
	}

	// destranca o print_mutex
	pthread_mutex_unlock(&philo->table->print_mutex);
}
