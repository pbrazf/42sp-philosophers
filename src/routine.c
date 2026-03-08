/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pedrferr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 23:13:52 by pedrferr          #+#    #+#             */
/*   Updated: 2026/02/21 23:13:52 by pedrferr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

bool	check_stop(t_table *table)
{
	bool	is_stopped;

	pthread_mutex_lock(&table->state_mutex);
	is_stopped = false;
	if (table->stop)
		is_stopped = true;
	pthread_mutex_unlock(&table->state_mutex);
	return (is_stopped);
}

static void	take_forks(t_philo *philo)
{
	if (philo->left_fork_idx < philo->right_fork_idx)
	{
		pthread_mutex_lock(&philo->table->forks[philo->left_fork_idx]);
		print_status(philo, "has taken a fork");
		pthread_mutex_lock(&philo->table->forks[philo->right_fork_idx]);
		print_status(philo, "has taken a fork");
	}
	else
	{
		pthread_mutex_lock(&philo->table->forks[philo->right_fork_idx]);
		print_status(philo, "has taken a fork");
		pthread_mutex_lock(&philo->table->forks[philo->left_fork_idx]);
		print_status(philo, "has taken a fork");
	}
}

static void	eat(t_philo *philo, t_rules *rules)
{
	long	time_now;

	pthread_mutex_lock(&philo->meal_lock);
	time_now = get_time_in_ms();
	philo->last_meal = time_now;
	philo->meals_eaten += 1;
	pthread_mutex_unlock(&philo->meal_lock);
	print_status(philo, "is eating");
	smart_sleep(rules->time_to_eat, philo->table);
}

static void	alone_philo_case(t_philo *philo)
{
	pthread_mutex_lock(&philo->table->forks[philo->left_fork_idx]);
	print_status(philo, "has taken a fork");
	smart_sleep(philo->table->rules.time_to_die, philo->table);
	pthread_mutex_unlock(&philo->table->forks[philo->left_fork_idx]);
}

void	*routine(void *arg)
{
	t_philo	*philo;
	t_rules	*rules;

	philo = (t_philo *)arg;
	rules = &philo->table->rules;
	if (philo->id % 2 == 0)
		usleep(rules->time_to_eat * 500);
	while (!check_stop(philo->table))
	{
		if (rules->num_philosophers == 1)
		{
			alone_philo_case(philo);
			return (NULL);
		}
		take_forks(philo);
		eat(philo, rules);
		pthread_mutex_unlock(&philo->table->forks[philo->left_fork_idx]);
		pthread_mutex_unlock(&philo->table->forks[philo->right_fork_idx]);
		print_status(philo, "is sleeping");
		smart_sleep(rules->time_to_sleep, philo->table);
		print_status(philo, "is thinking");
		if (rules->num_philosophers % 2 != 0)
			usleep(500);
	}
	return (NULL);
}
