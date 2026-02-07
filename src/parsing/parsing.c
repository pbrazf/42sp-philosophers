#include "philosophers.h"

static int check_argc(int argc)
{
	if (argc >= 5 && argc <= 6)
		return (1);
	return (0);
}

static bool	parse_int_pos(const char *s, int *out)
{
	long	n;
	int		d;

	if (!s || !*s || !out)
		return (false);
	n = 0;
	while (*s)
	{
		if (*s < '0' || *s > '9')
			return (false);
		d = *s - '0';
		if (n > (LONG_MAX - d) / 10)
			return (false);
		n = (n * 10) + d;
		s++;
	}
	if (n <= 0 || n > INT_MAX)
		return (false);
	*out = (int)n;
	return (true);
}

int validate_args(int argc, char *argv[], t_rules *rules)
{
	int	i;
	int value;
	
	if (!rules || !check_argc(argc))
		return (0);
	i = 1;
	while (i < argc)
	{
		if (!parse_int_pos(argv[i], &value))
			return (0);
		if (i == 1)
			rules->num_philosophers = value;
		if (i == 2)
			rules->time_to_die = value;
		if (i == 3)
			rules->time_to_eat = value;
		if (i == 4)
			rules->time_to_sleep = value;
		if (i == 5)
			rules->num_times_each_philosopher_must_eat = value;
		i++;
	}
	if (argc == 5)
		rules->num_times_each_philosopher_must_eat = -1;
	return (1);
}
