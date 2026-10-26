/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 23:16:34 by username          #+#    #+#             */
/*   Updated: 2026/10/21 23:40:25 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static char	*find_begin(char const *s1, char const *set)
{
	size_t	i;
	size_t	j;

	i = 0;
	while (s1[i])
	{
		j = 0;
		while (set[j] && s1[i] != set[j])
			j++;
		if (!set[j])
			break ;
		i++;
	}
	return ((char *) s1 + i);
}

static char	*find_end(char const *s1, char const *set, char const *begin)
{
	size_t	i;
	size_t	j;

	i = ft_strlen(s1);
	while (i > 0 && s1 + i - 1 >= begin)
	{
		j = 0;
		while (set[j] && s1[i - 1] != set[j])
			j++;
		if (!set[j])
			break ;
		i--;
	}
	if (i == 0 || s1 + i - 1 < begin)
		return ((char *) begin - 1);
	return ((char *) s1 + i - 1);
}

static char	*fill_str(char const *begin, char const *end)
{
	char	*new;
	size_t	len;
	size_t	i;

	len = end - begin + 1;
	new = malloc(sizeof(char) * (len + 1));
	if (!new)
		return (NULL);
	i = 0;
	while (i < len)
	{
		new[i] = begin[i];
		i++;
	}
	new[i] = '\0';
	return (new);
}

char	*ft_strtrim(char const *s1, char const *set)
{
	char	*begin;
	char	*end;

	if (!s1 || !set)
		return (NULL);
	begin = find_begin(s1, set);
	if (*begin == '\0')
		return (ft_strdup(""));
	end = find_end(s1, set, begin);
	return (fill_str(begin, end));
}
