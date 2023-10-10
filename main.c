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

#include "minishell.h"

//used to print out the token types (only for testing purposes)
const char *token_type_names[] = 
{
	"WORD",
	"WHITESPACE",
    "PIPE",
    "REDIRECT_HEREDOC",
    "REDIRECT_APPEND",
    "REDIRECT_IN",
    "REDIRECT_OUT",
	"DOUBLE_QUOTES",
    "SINGLE_QUOTES",
	"ENV",
	"ENV_FAIL",
};

//only for testing purposes, prints a list
void	printlist(t_list *head)
{
	t_list	*temporary;
	t_token *token;
	int i;

	i = 0;
	temporary = head;
	while (temporary != NULL)
	{
		token = (t_token *)temporary->value;
		printf("list[%d]: %s type: %s\n", i, token->str, token_type_names[token->type]); // casted to char since in the first test we want to print a word
		temporary = temporary->next;
		i++;
	}
}

/* determine the size of a list */
int	lst_size(t_list *head)
{
	int		lsize;
	t_list	*current_node_pos;

	lsize = 0;
	current_node_pos = head;
	while (current_node_pos != NULL)
	{
		current_node_pos = current_node_pos->next;
		lsize++;
	}
	return (lsize);
}

/* returns the value of the last node in a list */
t_list	*return_tail_value(t_list *head)
{
	t_list	*current_node_pos;
	int		lsize;

	current_node_pos = head;
	lsize = lst_size(head);
	while (lsize > 1)
	{
		current_node_pos = current_node_pos->next;
		lsize--;
	}
	return (current_node_pos);
}

/* creates a new node (that is to be added to a list in subsequent function) */
t_list	*create_new_node(void *value)
{
	t_list	*newnode;

	newnode = malloc(sizeof(t_list));
	if (newnode == NULL)
		return (NULL);
	newnode->value = value;
	newnode->next = NULL;
	return (newnode);
}

/* inserts a node at the end of a list */
void	insert_at_tail(t_list *head, t_list *new_value)
{
	t_list	*current;

	if (head == 0 || new_value == 0)
		return ;
	current = head;
	while (current->next != NULL)
		current = current->next;
	current->next = new_value;
}


t_token *add_token_type_and_str(char *str_with_all_tokens, t_type token_type)
{
	t_token *token;

	token = malloc(sizeof(t_token));
	if (!token)
		return(NULL);
	token->str = str_with_all_tokens;
	token->type = token_type;
	return(token);
}

/* adds a new node to a list; in case the list is empty, the new node becomes the head, else 
it is added at the end of the list */
t_list *add_token_to_list(t_list **token_list, char *str_with_all_tokens, t_type token_type)
{
	t_list *new_node;
	t_token *data;

	data = add_token_type_and_str(str_with_all_tokens, token_type);
	if (!data)
		return (NULL);
	new_node = create_new_node(data);
	if (!*token_list)
		*token_list = new_node;
	else
		insert_at_tail(*token_list, new_node);
	return (*token_list);
}

bool	check_for_metacharacter(char c)
{
	if (c == ' ' || c == '\t' || c == '\n' || c == '|' || c == '<' || c == '>')
		return (true);
	else
		return (false);
}

/* checks if c is a double or single quote */
bool	check_for_quotes(char c)
{
	if (c == '"' || c == '\'')
		return (true);
	else
		return (false);
}

/* function that gets called in case a redirection token was encountered in the input string;
determines the redirection type depending on the input and returns a duplicate */
char *redirection_token(char *line, int *i, t_type *token_type)
{
	if (line[*i] == '<' && line[*i + 1] == '<')
	{
		*token_type = REDIRECT_HEREDOC;
		(*i) += 2;
		return (ft_strdup("<<"));
	}
	else if (line[*i] == '>' && line[*i + 1] == '>')
	{
		*token_type = REDIRECT_APPEND;
		(*i) += 2;
		return (ft_strdup(">>"));
	}
	else if (line[*i] == '>') //! not detected correctly
	{
		*token_type = REDIRECT_OUT;
		(*i) += 1;
		return (ft_strdup(">"));
	}
	else
	{
		*token_type = REDIRECT_IN;
		(*i) += 1;
		return (ft_strdup("<"));
	}
}

/* function that gets called when a pipe token was detected in the input string; it sets the 
token type and returns a duplicate */
char *pipe_token(int *i, t_type *token_type)
{
	*token_type = PIPE;
	(*i)++;
	return(ft_strdup("|"));
}

/* each substring is malloced! */
char *check_for_word_token(char *line, int *i, t_type *token_type)
{
	int start_index;
	int end_index;

	*token_type = WORD;
	start_index = *i;
	while (check_for_metacharacter(line[*i]) == false && check_for_quotes(line[*i]) == false && line[*i] != '$' && line[*i])
		(*i)++;
	end_index = *i - start_index;
	return(ft_substr(line, start_index, end_index));
}

char *whitespace_token(char *line, int *i, t_type *token_type)
{
	int start_index;
	int end_index;

	*token_type = WHITESPACE;
	start_index = *i;
	while ((line[*i] == ' ' || line[*i] == '\t') && line[*i])
		(*i)++;
	end_index = *i - start_index;
	return(ft_substr(line, start_index, end_index));
}

//! this should be the expand function check for 
char *env_within_double_quotes(char *line, int *i)
{
	int start;
	int length;
	char *env_string;

	start = ++(*i); // pre-iterate since index is still at '$'
	if (line[*i] == '\0' || line[*i] == '"')
		return(ft_strdup("$"));
	while (line[*i] && check_for_metacharacter(line[*i]) == false && line[*i] != '\'' && line[*i] != '"' && line[*i] != '$')
		(*i)++;
	length = *i - start;
	env_string = malloc(sizeof(char) * (length + 1)); //! MALLOC
	if (!env_string)
		return (NULL);
	strncpy(env_string, &line[start], length);
	env_string[length] = '\0';
	//printf("%s", env_string);
	return(env_string);
}

char	*append_str(char *str, char *appendix)
{
	char	*appended_str;

	if (!str)
		return(appendix);
	appended_str = ft_strjoin(str, appendix);
	//free(str);
	//free(appendix);
	return(appended_str);
}

char	*char_to_str(char c)
{
	char	*str;
	int		i;

	str = malloc(sizeof(char) * 2); //!MALLOC
	if (str == NULL)
		return (NULL);
	i = 0;
	str[i] = c;
	str[++i] = '\0';
	return(str);
}

char *double_quote_to_string(char *line, int *i, t_minishell m)
{
	char *str_between_quotes;
	char	*search_str;
	char	*env_expanded;

	str_between_quotes = NULL;
	while (line[*i] != '"' && line[*i] != '\0')
	{
		if (line[*i] == '$')
		{	
			(*i)++;
			search_str = extract_env_name(line, i);
			if (check_if_part_of_library(m.env_lib, search_str) == true)
				env_expanded = *find_path(m.envp_lib, search_str);
			else if (check_if_part_of_library(m.env_lib, search_str) == false)
				env_expanded = "";
			str_between_quotes = append_str(str_between_quotes, env_expanded);
			free(search_str);
		}
		else
			str_between_quotes = append_str(str_between_quotes, char_to_str(line[(*i)++]));
	}
	if (line[*i])
		(*i)++;
	return(str_between_quotes);
}

char *single_quote_to_string(char *line, int *i)
{
	char *str_between_quotes;
	int start;
	int length;

	start = *i;
	length = 0;
	while (line[*i] && line[*i] != '\'')
		(*i)++;
	length = *i - start;
	str_between_quotes = malloc(sizeof(char) * (length + 1)); //! MALLOC
	if (!str_between_quotes)
		return (NULL);
	strncpy(str_between_quotes, &line[start], length);
	str_between_quotes[length] = '\0';
	if (line[*i])
		(*i)++;
	return(str_between_quotes);
}

char *single_or_double_quotes_token(char *line, int *i, t_type *token_type, t_minishell m)
{
	if (line[*i] == '"')
	{
		*token_type = WORD;//DOUBLE_QUOTES; //!so in my version i don't even need this?!
		if (line[*i])
			(*i)++;
		return(double_quote_to_string(line, i, m));
	}
	else
	{
		*token_type = WORD;//SINGLE_QUOTES;
		if (line[*i])
			(*i)++;
		return(single_quote_to_string(line, i));
	}
}

t_list *find_previous_node(t_list *head, t_list *target_node)
{
    t_list *previous_node;
    t_list *current_node;
	
	current_node = head;
	previous_node = NULL;
    while (current_node != NULL && current_node != target_node)
    {
        previous_node = current_node;
        current_node = current_node->next;
    }
    return (previous_node);
}

void	ft_lstremove(t_list **lst, t_list *node, void (*del)(void *))
{
	t_list	*prev;

	if (!lst || !(*lst) || !node)
		return ;
	prev = find_previous_node(*lst, node);
	if (!prev)
		*lst = node->next;
	else
		prev->next = node->next;
	ft_lstdelone(node, del);
}

/* helper function for deleting a token and freeing its memory */
void token_del(void *content) {
    if (content == NULL) {
        return;
    }

    t_token *token = (t_token *)content;
    if (token->str != NULL) {
        free(token->str); // Assuming the str field is dynamically allocated.
    }
    
    free(token); // Free the token structure itself.
}

/* used for joining two consecutive word tokens; the function takes in a pointer to the head of the
list and two pointers to the current node and the previous node;
if the token type of the previous node is WORD the string contents of the two consecutive tokens are joined 
into one and one token is freed */
void join_str_and_del_old_node(t_list *tlist, t_list *current_node, t_list *previous_node)
{
	char	*new_joined_str;
	t_token *previous_token;

	previous_token = previous_node->value;
	if (previous_token->type == WORD)
	{
		new_joined_str = ft_strjoin(previous_token->str, ((t_token *)current_node->value)->str); 
		free(((t_token *)previous_node->value)->str);
		((t_token *)previous_node->value)->str = new_joined_str;
		ft_lstremove(&tlist, current_node, token_del);
	}
}

/* function that merges two consecutive word tokens (without spaces in between!)
e.g. l"s"; otherwise the parser would process this as two separate tokens */
t_list	*merge_words(t_list **tlist, t_list *current_node)
{
	t_list *previous_node;
	t_token	*current_token;

	previous_node = NULL;
	current_token = current_node->value;
	if (current_token->type == WORD)
	{
		previous_node = find_previous_node(*tlist, current_node);
		if (previous_node != NULL)
			join_str_and_del_old_node(*tlist, current_node, previous_node);
	}
	return (*tlist);
}

t_list	*delete_whitespace(t_list **tlist, t_list *current_node)
{
	t_token	*tmp_token;

	tmp_token = current_node->value;
	if (tmp_token->type == WHITESPACE)
		ft_lstremove(tlist, current_node, token_del);
	return(*tlist);
}

t_list	*modify_list(t_list **tlist, t_list *(*f)(t_list **tlist, t_list *current_node))
{
	t_list	*current_node;
	t_list	*next;

	current_node = *tlist;
	while (current_node)
	{
		next = current_node->next;
		*tlist = (*f)(tlist, current_node);
		current_node = next;
	}
	return (*tlist);
}

t_list	*cleanup_token_list(t_list *tlist)
{
	tlist = modify_list(&tlist, merge_words);
	tlist = modify_list(&tlist, delete_whitespace);
	return(tlist);
}

//! does not work because dict list is not set!!!
char	*get_env_value(t_list *dict, char *key, char **envp)
{
	///t_dict	*item;
	int	i;
	(void)dict;
	i = 0;
	if (!key)
		return (NULL);
/* 	while (dict)
	{
		//item = (t_dict *)dict->value;
 		if (!ft_strcmp(item->key, key))
			return (item->value);
		dict = dict->next;
	} */
	while (envp[i] != NULL)
	{
		if (envp[i] == key)
			return(envp[i]);
		i++;
	}
	return (NULL);
}

char **create_env_library(char **envp)
{
    char **buf = NULL;
    char target = '=';
    int len = 0;
    int substr_len;

    while (envp[len] != NULL)
        len++;
    buf = malloc(sizeof(char *) * (len + 1));
    if (!buf)
        return (NULL);
    len = 0; //! Don't get that logic fully 
    while (envp[len] != NULL)
    {
        char *target_pos = strchr(envp[len], target); //! LIBFT
        if (target_pos != NULL)
        {
            substr_len = target_pos - envp[len];
            buf[len] = malloc(substr_len + 1);
            if (!buf[len])
                return (NULL);
            strncpy(buf[len], envp[len], substr_len); //! LIBFT
            buf[len][substr_len] = '\0';
            //printf("SUBSTR: %s\n", buf[len]);
        }
        else
        {
            buf[len] = NULL;
        }
        len++;
    }
    buf[len] = NULL;
    return (buf);
}

char **create_envp_library(char **envp)
{
    char **buf = NULL;
    char target = '\0';
    int len = 0;
    int substr_len;

    while (envp[len] != NULL)
        len++;
    buf = malloc(sizeof(char *) * (len + 1));
    if (!buf)
        return (NULL);
    len = 0; //! Don't get that logic fully 
    while (envp[len] != NULL)
    {
        char *target_pos = strchr(envp[len], target); //! LIBFT
        if (target_pos != NULL)
        {
            substr_len = target_pos - envp[len];
            buf[len] = malloc(substr_len + 1);
            if (!buf[len])
                return (NULL);
            strncpy(buf[len], envp[len], substr_len); //! LIBFT
            buf[len][substr_len] = '\0';
            //printf("SUBSTR: %s\n", buf[len]);
        }
        else
        {
            buf[len] = NULL;
        }
        len++;
    }
    buf[len] = NULL;
    return (buf);
}

bool check_if_part_of_library(char **env_lib, char *search_str)
{
	int	i;

	i = 0;
	while (env_lib[i] != NULL)
	{
		if (ft_strcmp(env_lib[i], search_str) == 0)
			return(true);
		i++;
	}
	return(false);
}


/* searches for the corresponding env to search_str within envp;
when assigning to the buffer path is iterated by one to skip the equal sign*/
char	**find_path(char **envp, char *search_str)
{
	int		i;
	char	*path;
	char	**path_buf;
	
	i = 0;
	while (ft_strnstr(envp[i], search_str, ft_strlen(search_str)) == NULL)
		i++;
	path = ft_strstr(envp[i], "=");
	if (path == NULL)
		return (NULL);
	path_buf = ft_split(++path, '\0');
	return (path_buf);
}

/* search_str is the string that needs to be found */
char	*extract_env_name(char *line, int *i)
{
	int	start;
	int length;
	char	*search_str;

	start = *i;
	if (!line[*i] || line[*i] == '"')
		return(ft_strdup("$"));
	//! add g exit code
	while(line[*i] != '\0' && check_for_metacharacter(line[*i]) == false &&
	check_for_quotes(line[*i]) == false && line[*i] != '$')
		(*i)++;
	length = *i - start;
	search_str = ft_substr(line, start, length);
	return (search_str);
}

/* extract the env_str from the input */
char	*env_token(char *line, int *i, t_type *token_type, char **env_lib, char **envp)
{
	char	*search_str;
	char	*env_final;
	//char	**env_lib;

	(*i)++;
	*token_type = WORD;//ENV;
	search_str = extract_env_name(line, i);
	printf("SEARCH: %s", search_str);
	//env_lib = create_env_library(envp);
	if (check_if_part_of_library(env_lib, search_str) == false)
	{
		*token_type = ENV_FAIL;
		if (ft_strcmp(search_str, "$") == 0) // if theres a single dollar sign it's supposed to be printed
		{
			*token_type = WORD;//not needed anymore
			free(search_str);
			return(ft_substr("$", 0, 1));
		}
		free(search_str);
		return (NULL); //! does this make sense? how should I process it?
		//! if no matching ENV nothing happens, meaning I could do if type ENV_FAIL = skip that command
	}
	else 
	{
		env_final = *find_path(envp, search_str);
		free(search_str);
	}
	return (env_final);
}

// add multiple checks for all kind of delimiters e.g. parameter, quotes, whitespaces
t_list *split_line_into_tokens(t_minishell m, char **envp)
{
	int 	i;

	i = 0;
	m.tlist = NULL;
	while (m.line[i])
	{
		if (m.line[i] == '|')
			m.str_with_all_tokens = pipe_token(&i, &m.token_type);
		else if (m.line[i] == '<' || m.line[i] == '>')
			m.str_with_all_tokens = redirection_token(m.line, &i, &m.token_type);
		else if (m.line[i] == '$')
			m.str_with_all_tokens = env_token(m.line, &i, &m.token_type, m.env_lib, envp);
		else if (m.line[i] == ' ' || m.line[i] == '\t')
			m.str_with_all_tokens = whitespace_token(m.line, &i, &m.token_type);
		else if (m.line[i] == '\'' || m.line[i] == '\"')
			m.str_with_all_tokens = single_or_double_quotes_token(m.line, &i, &m.token_type, m);
		else
			m.str_with_all_tokens = check_for_word_token(m.line, &i, &m.token_type);
		m.tlist = add_token_to_list(&m.tlist, m.str_with_all_tokens, m.token_type);
	}
	cleanup_token_list(m.tlist);
	return(m.tlist);
}

//! SEGFAULTS AT EXIT
/* to free up all memory in the end, including memory automatically allocated by readline function */
void	freememory(t_minishell m)
{
	t_list	*delete;

	while (lst_size(m.tlist) > 0)
	{
		delete = m.tlist;
		m.tlist = m.tlist->next;
		free(delete);
	}
	free(m.line);
}

void	*ft_free_set_null(void *ptr)
{
	if (ptr)
		free(ptr);
	return (NULL);
}

//! DO WE NEED TO REPLICATE BASH EXIT CODES?
int	exit_shell(t_minishell m)
{
	printf("\nExiting...\n");
	if (m.line)
		m.line = ft_free_set_null(m.line);
	if (m.tlist)
		ft_lstclear(&m.tlist, token_del);
	/* if (m.clist)
		ft_lstclear(&m.clist, command_del);
	 */
	//freememory(m);
	exit(1);
}

/* signal handler function that is designed to take in SIGINT signal, which is generated
when the user presses Ctrl+C in the terminal; If the signal is received the handler prints
a new line, which effectively means that it moves to the next line */
void	handle_signals(int signal)
{
	if (signal == SIGINT)
	{
		printf("\n");
		rl_on_new_line();
		//rl_replace_line("", 0); //!not working on mac!!!
		rl_redisplay();
	}
}

/*
Signals that need to be implemented similar to their bash behavior:
Ctrl+C -> move to a new line in the terminal (sends SIGINT signal),
Ctrl+D -> exit the shell,
Ctrl+\ -> terminate process and generate a core dump (sends SIGQUIT signal),
In interactive mode:
◦ ctrl-C displays a new prompt on a new line. 
◦ ctrl-D exits the shell.
◦ ctrl-\ does nothing.

SA_RESTART tells the operating system to restart system calls that were interrupted by a signal
rather than returning an error;
whenever a SIGINT signal is received, the handle function is called;
the last line sets the behavior of the SIGQUIT signal to be ignored;
*/
void	init_signals(t_minishell	*m, void (*handle)(int))
{
	m->sa.sa_handler = handle;
	m->sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &m->sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}

/* first initialize all fields in the main data structure t_minishell to zero by using memset;
create libaries for the envp fields, one containing the full information and one only the 
parameter name, which is later being used as a search keyword */
void	init_minishell_struct_and_signals(t_minishell *m, char **envp)
{
	ft_memset(m, 0, sizeof(t_minishell));
	m->env_lib = create_env_library(envp);
	m->envp_lib = create_envp_library(envp);
	init_signals(m, handle_signals);
}

/* shell is only created if there is exactly one argument (name of the executable);
m.line == NULL to exit if the user calls Ctrl+D or simply if "exit" is called;
tlist = tokenlist, meaning the list that holds all tokens,
clist = commandlist, meaning the list that holds all commands */
int main(int ac, char **av, char **envp)
{
	t_minishell m;

	(void)av;
	if (ac != 1)
		return (1);
	init_minishell_struct_and_signals(&m, envp);
	while(1)
	{
		m.line = readline("Myshell: ");
		if (!m.line)
			exit_shell(m);
		add_history(m.line);
		if (m.line == NULL || ft_strcmp(m.line, "exit") == 0) 
			exit_shell(m);
		m.tlist = split_line_into_tokens(m, envp);
		printlist(m.tlist); //! only for testing
		m.clist = parser(m);
		//! TODO: FREE TOKEN & COMMAND LIST
		free(m.line);
	}
}
