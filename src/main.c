/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pedrferr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 17:54:31 by pedrferr          #+#    #+#             */
/*   Updated: 2026/02/22 01:15:10 by pedrferr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

int	main(int argc, char *argv[])
{
	t_rules rules;
	t_table table;

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
	// cleanup_table(&table); // TODO: Implement proper cleanup later
	return (1);
}
