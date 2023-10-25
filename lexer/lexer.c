/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amaucher <amaucher@student.42berlin.d      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/23 10:13:39 by amaucher          #+#    #+#             */
/*   Updated: 2023/08/23 10:13:42 by amaucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

/*
iterates through line and splits it into tokens
*/
t_list	*split_line_into_tokens(t_minishell m)
{
	int	i;

	i = 0;
	m.tlist = NULL;
	while (m.line[i])
	{
		if (m.line[i] == '|')
			m.token_str = pipe_token(&i, &m.token_type);
		else if (m.line[i] == '<' || m.line[i] == '>')
			m.token_str = redirection_token(m.line, &i, &m.token_type);
		else if (m.line[i] == '$')
			m.token_str = env_token(m.line, &i, &m.token_type, m.envp);
		else if (m.line[i] == ' ' || m.line[i] == '\t')
			m.token_str = whitespace_token(m.line, &i, &m.token_type);
		else if (m.line[i] == '\'' || m.line[i] == '\"')
			m.token_str = quotes(m.line, &i, &m.token_type, m);
		else
			m.token_str = check_for_word_token(m.line, &i, &m.token_type);
		m.tlist = add_token_to_list(&m.tlist, m.token_str, m.token_type);
	}
	cleanup_token_list(m.tlist);
	return (m.tlist);
}
