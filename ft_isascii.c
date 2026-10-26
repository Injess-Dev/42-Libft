/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isascii.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 14:45:37 by username          #+#    #+#             */
/*   Updated: 2026/10/19 14:50:11 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_isascii(int C)
{
	if (C >= 0 && C <= 127)
		return (1);
	else
		return (0);
}

// #include "stdio.h"

// int	main(void)
// {
// 	printf("%d \n", ft_isascii('d'));
// }
