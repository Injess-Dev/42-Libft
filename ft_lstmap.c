/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcardon <tcardon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 05:33:43 by username          #+#    #+#             */
/*   Updated: 2026/10/22 05:37:16 by tcardon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*start;
	t_list	*current;
	void	*content;

	if (!f || !del)
		return (NULL);
	start = NULL;
	while (lst)
	{
		content = f(lst->content);
		current = ft_lstnew(content);
		if (!current)
		{
			del(content);
			ft_lstclear(&start, del);
			return (NULL);
		}
		if (!start)
			start = current;
		else
			ft_lstlast(start)->next = current;
		lst = lst->next;
	}
	return (start);
}
