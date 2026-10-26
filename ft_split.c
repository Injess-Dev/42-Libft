/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 16:39:48 by username          #+#    #+#             */
/*   Updated: 2026/10/21 16:52:00 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	count_words(char const *s, char c)
{
	size_t	words;
	size_t	i;

	words = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] != c && (s[i + 1] == c || s[i + 1] == '\0'))
			words++;
		i++;
	}
	return (words);
}

static void	free_tab(char **tab, size_t i)
{
	while (i > 0)
	{
		i--;
		free(tab[i]);
	}
	free(tab);
}

static char	*dup_word(char const *s, char c)
{
	size_t	len;

	len = 0;
	while (s[len] && s[len] != c)
		len++;
	return (ft_substr(s, 0, len));
}

static int	set_mem(char **tab, char const *s, char c)
{
	size_t	index;
	size_t	i;

	index = 0;
	i = 0;
	while (s[index])
	{
		if (s[index] != c)
		{
			tab[i] = dup_word(s + index, c);
			if (!tab[i])
			{
				free_tab(tab, i);
				return (0);
			}
			i++;
			while (s[index] && s[index] != c)
				index++;
		}
		else
			index++;
	}
	tab[i] = NULL;
	return (1);
}

char	**ft_split(char const *s, char c)
{
	char	**tab;

	tab = malloc(sizeof(char *) * (count_words(s, c) + 1));
	if (!tab)
		return (NULL);
	if (!set_mem(tab, s, c))
		return (NULL);
	return (tab);
}
