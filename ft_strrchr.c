/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 15:09:46 by username          #+#    #+#             */
/*   Updated: 2026/10/21 16:19:30 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strrchr(const char *str, int occ)
{
	char	*last;
	size_t	i;

	last = NULL;
	i = 0;
	while (str[i] != '\0')
	{
		if (str[i] == (char) occ)
			last = (char *) &str[i];
		i++;
	}
	if ((char) occ == '\0')
		return ((char *) &str[i]);
	return (last);
}
