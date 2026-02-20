/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pedrferr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 17:54:31 by pedrferr          #+#    #+#             */
/*   Updated: 2026/02/19 21:50:56 by pedrferr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

int	main(int argc, char *argv[])
{
	t_rules rules;
	t_table table;

	if (!validate_args(argc, argv, &rules))
	{
		write(1, "Error args!\n", 12);
		return (0);
	}
	table.rules = rules;
	if (!init_table(&table))
	{
		write(1, "Error init!\n", 12);
		return (0);
	}
	write(1, "Success init!\n", 14);
	// cleanup_table(&table); // TODO: Implement proper cleanup later
	return (1);
}
