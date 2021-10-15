/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcollas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/26 18:09:22 by rcollas           #+#    #+#             */
/*   Updated: 2021/10/09 16:25:58 by rcollas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/pipex.h"

int	infile_cmd(t_var *var, int **pipefd, int i)
{
	char	**cmd_args;
	int		j;

	j = -1;
	cmd_args = ft_split(var->av[i + 2], ' ');
	if (var->file1 >= 0)
		dup2(var->file1, STDIN_FILENO);
	dup2(pipefd[i + 1][1], STDOUT_FILENO);
	close_pipes(pipefd, var);
	if (var->file1 >= 0)
		close(var->file1);
	if (access(var->cmds[i], X_OK) == 0)
		execve(var->cmds[i], cmd_args, NULL);
	free_arg(cmd_args);
	return (1);
}

int	in_between_cmds(t_var *var, int **pipefd, int i)
{
	char	**cmd_args;
	int		j;

	cmd_args = ft_split(var->av[i + 2], ' ');
	j = -1;
	dup2(pipefd[i][0], STDIN_FILENO);
	dup2(pipefd[i + 1][1], STDOUT_FILENO);
	close_pipes(pipefd, var);
	if (access(var->cmds[i], X_OK) == 0)
		execve(var->cmds[i], cmd_args, NULL);
	free_arg(cmd_args);
	return (1);
}

int	outfile_cmd(t_var *var, int **pipefd, int i)
{
	char	**cmd_args;
	int		j;

	j = -1;
	cmd_args = ft_split(var->av[i + 2], ' ');
	dup2(var->file2, STDOUT_FILENO);
	dup2(pipefd[i][0], STDIN_FILENO);
	close_pipes(pipefd, var);
	if (access(var->cmds[i], X_OK) == 0)
		execve(var->cmds[i], cmd_args, NULL);
	free_arg(cmd_args);
	return (1);
}

int	proceed_pipes(t_var *var, int **pipefd, int i)
{
	if (i == 0)
	{
		if (infile_cmd(var, pipefd, i) == FAIL)
			return (0);
	}
	else if (i == var->size - 2)
		outfile_cmd(var, pipefd, i);
	else
		in_between_cmds(var, pipefd, i);
	return (1);
}

int	exec(t_var *var, int **pipefd, pid_t *pids)
{
	int		i;

	i = -1;
	if ((var->file1 = open(var->av[1], O_RDONLY)) < 0)
		perror(var->av[1]);
	if ((var->file2 = open(var->av[var->ac - 1], O_CREAT | O_RDWR | O_TRUNC, 0644)) < 0)
		perror(var->av[1]);
	while (++i < var->size - 1)
	{
		pids[i] = fork();
		if (pids[i] == -1)
		{
			perror("Fork failed:");
			return (0);
		}
		if (pids[i] == 0)
		{
			if (proceed_pipes(var, pipefd, i) == FAIL)
				return (0);
		}
	}
	close_pipes(pipefd, var);
	i = -1;
	while (++i < var->size - 1)
		waitpid(pids[i], 0, 0);
	return (1);
}
