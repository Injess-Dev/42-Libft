/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 18:27:54 by username          #+#    #+#             */
/*   Updated: 2026/10/20 19:27:17 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memset(void *s, int c, size_t n)
{
	unsigned char	*i;

	i = (unsigned char *) s;
	while (n > 0)
	{
		*i = (unsigned char) c;
		i++;
		n--;
	}
	return (s);
}

// #include "stdio.h"

// int	main(void)
// {
// 	char	str[100] = "Libc is the standard library.";

// 	ft_memset(str, '.', 8 * sizeof(char));
// 	printf("After memset(): %s\n", str);
// 	return (0);
// }
