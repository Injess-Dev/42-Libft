/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_toupper.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 15:15:48 by username          #+#    #+#             */
/*   Updated: 2026/10/19 18:45:35 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_toupper(int character)
{
	if (character >= 97 && character <= 122)
		character = character - 32;
	return (character);
}

// #include <stdio.h>

// int	main(void)
// {
// 	printf("%d", ft_toupper('z'));
// }
