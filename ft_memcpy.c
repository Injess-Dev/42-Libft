/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcpy.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:28:20 by username          #+#    #+#             */
/*   Updated: 2026/10/20 19:51:42 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memcpy(void *destination, const void *source, size_t size)
{
	size_t				i;
	unsigned char		*d;
	const unsigned char	*s;

	if (!destination && !source)
		return (NULL);
	i = 0;
	d = (unsigned char *) destination;
	s = (const unsigned char *) source;
	while (i < size)
	{
		d[i] = s[i];
		i++;
	}
	return (destination);
}

// #include "stdio.h"

// int	main(void)
// {
// 	char	destination[100] = "42";
// 	char	source[100] = "Ecole 42 projet libft";

// 	ft_memcpy(destination, source, sizeof(source));
// 	printf("%s", destination);
// 	return (0);
// }
