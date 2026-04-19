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

static int	ft_strcmp(char *str1, char *str2)
{
	int	i;

	i = 0;
	while (str1[i] && str2[i] && str1[i] == str2[i])
		i++;
	return (str1[i] - str2[i]);
}

void	print_status(t_philo *philo, char *status)
{
	long	time_now;

	pthread_mutex_lock(&philo->table->print_mutex);
	if (!check_stop(philo->table) || !ft_strcmp(status, "died"))
	{
		time_now = get_time_in_ms() - philo->table->start_ms;
		printf("%ld %d %s\n", time_now, philo->id, status);
	}
	pthread_mutex_unlock(&philo->table->print_mutex);
}
