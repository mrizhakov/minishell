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

# include "../minishell.h"

/*
unset function is used to unset or remove environment variables or
shell variables
Syntax: 'unset variable_name'
Exit Status: The unset command typically returns a status of 0 if the variable
is successfully unset. If the variable does not exist, it still returns 0.
If multiple variable names are entered after unset, all will be checked and
unset even if incorrect variable names are included.
*/

int unset(t_minishell m, t_command *cmd)
{
    int i,

    i = 1;
    while(cmd->args[i] != NULL)
    {
        check_if_part_of_library((m.env_lib, cmd->args[i]) == true)
        {
            
        }

        
    }
}