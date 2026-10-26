/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 15:32:29 by username          #+#    #+#             */
/*   Updated: 2026/10/21 15:37:05 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	*ft_memmalloc(size_t size)
{
	void	*new;
	size_t	i;

	new = (void *) malloc(size);
	if (!new)
		return (NULL);
	i = 0;
	while (i < size)
	{
		((unsigned char *) new)[i] = 0;
		i++;
	}
	return (new);
}

void	*ft_calloc(size_t count, size_t size)
{
	if (size != 0 && count > ((size_t) - 1 / size))
		return (NULL);
	return (ft_memmalloc(count * size));
}
