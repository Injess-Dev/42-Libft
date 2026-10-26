/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 21:31:10 by username          #+#    #+#             */
/*   Updated: 2026/10/20 22:26:02 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *dest, const void *src, size_t n)
{
	char	*d;
	char	*s;
	size_t	i;

	d = (char *) dest;
	s = (char *) src;
	i = 0;
	if (d == s)
		return (d);
	if (s < d)
	{
		i = n;
		while (i--)
			((char *) d)[i] = ((char *) s)[i];
	}
	else
	{
		i = 0;
		while (i < n)
		{
			((char *) d)[i] = ((char *) s)[i];
			i++;
		}
	}
	return (d);
}

// #include "stdio.h"

// int	main(void)
// {
// 	char	destination[100] = "test test test";
// 	char	source[100] = "Ecole 42 libft projet";

// 	ft_memmove(destination, source, 10);
// 	printf("%s", destination);
// 	return (0);
// }
