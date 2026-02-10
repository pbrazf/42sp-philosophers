#include "philosophers.h"

int	main(int argc, char *argv[])
{
	t_rules rules;

	if (!validate_args(argc, argv, &rules))
	{
		write(1, "Error!\n", 7);
		return (0);
	}
	write(1, "Success!\n", 9);
	return (1);
}
