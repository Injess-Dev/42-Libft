/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isprint.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 14:50:54 by username          #+#    #+#             */
/*   Updated: 2026/10/20 14:47:56 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_isprint(int C)
{
	if (C >= 32 && C <= 126)
		return (1);
	else
		return (0);
}

// #include "stdio.h"

// int	main(void)
// {
// 	printf("%d", ft_isprint('J'));
// }
