/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_bzero.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:59:53 by username          #+#    #+#             */
/*   Updated: 2026/10/20 18:17:53 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_bzero(void *s, size_t n)
{
	size_t			i;
	unsigned char	*str;

	str = (unsigned char *) s;
	i = 0;
	while (i < n)
	{
		str[i] = 0;
		i++;
	}
}

// #include <stdio.h>

// int	main(void)
// {
// 	char	str[] = "Bonjour";

// 	printf("Avant ft_bzero : %s\n", str);
// 	ft_bzero(str, 3);
// 	printf("Apres ft_bzero : ");
// 	for (size_t i = 0; i < sizeof(str); i++)
// 	{
// 		printf("%d ", (unsigned char) str[i]);
// 	}
// 	printf("\n");
// 	return (0);
// }
