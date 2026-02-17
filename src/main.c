/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pedrferr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 17:54:31 by pedrferr          #+#    #+#             */
/*   Updated: 2026/02/17 17:54:36 by pedrferr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
