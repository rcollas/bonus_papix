/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_bonus.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcollas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/26 18:09:22 by rcollas           #+#    #+#             */
/*   Updated: 2021/10/20 10:30:47 by rcollas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/pipex_bonus.h"

int	infile_cmd(t_var *var, int **pipefd, int i)
{
	char	**cmd_args;
	int		j;

	j = -1;
	cmd_args = ft_split(var->av[i + 2], ' ');
	dup2(var->file1, STDIN_FILENO);
	dup2(pipefd[i + 1][1], STDOUT_FILENO);
	close_pipes(pipefd, var);
	close(var->file1);
	if (execve(var->cmds, cmd_args, NULL) == FAIL)
	{
		perror("Execve failed:");
		return (free_arg(cmd_args));
	}
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
	if (execve(var->cmds, cmd_args, NULL) == FAIL)
	{
		perror("Execve failed:");
		return (free_arg(cmd_args));
	}
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
	if (execve(var->cmds, cmd_args, NULL) == FAIL)
	{
		perror("Execve failed:");
		return (free_arg(cmd_args));
	}
	free_arg(cmd_args);
	return (1);
}

void	proceed_pipes(t_var *var, int **pipefd, int i)
{
	if (i == 0)
	{
		if (var->file1 < 0)
			return (perror(var->av[1]));
		if (get_cmds(var, i) == FAIL)
			return ;
		infile_cmd(var, pipefd, i);
	}
	else if (i == var->size - 2)
	{
		if (var->file2 < 0)
			return (perror(var->av[var->size + 1]));
		if (get_cmds(var, i) == FAIL)
			return ;
		outfile_cmd(var, pipefd, i);
	}
	else
	{
		if (get_cmds(var, i) == FAIL)
			return ;
		in_between_cmds(var, pipefd, i);
	}
}

int	exec(t_var *var, int **pipefd, pid_t *pids)
{
	int		i;
	int		status;

	i = -1;
	var->file1 = open(var->av[1], O_RDONLY);
	var->file2 = open(var->av[var->ac - 1], O_CREAT | O_RDWR | O_TRUNC, 0644);
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
			proceed_pipes(var, pipefd, i);
			break ;
		}
	}
	close_pipes(pipefd, var);
	i = -1;
	while (++i < var->size - 2)
		waitpid(pids[i], &status, 0);
	return (1);
}