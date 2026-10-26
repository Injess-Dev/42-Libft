/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_tolower.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 18:43:52 by username          #+#    #+#             */
/*   Updated: 2026/10/19 18:47:42 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_tolower(int character)
{
	if (character >= 65 && character <= 90)
		character = character + 32;
	return (character);
}

// #include <stdio.h>

// int	main(void)
// {
// 	printf("%d", ft_tolower('Z'));
// }
