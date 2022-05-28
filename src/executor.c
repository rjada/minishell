/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rjada <rjada@student.21-school.ru>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/27 13:35:04 by rjada             #+#    #+#             */
/*   Updated: 2022/05/28 23:05:36 by rjada            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

static char	**create_paths(char *cmd, char **envp)
{
	int		i;
	char	**paths;
	char	**full_paths;

	i = 0;
	paths = NULL;
	while (envp[i])
	{
		if (ft_strncmp(envp[i], "PATH=", 5) == 0)
			paths = ft_split(envp[i] + 5, ':');
		i++;
	}
	if (!paths)
		return (NULL);
	i = 0;
	while (paths[i])
		i++;
	full_paths = malloc(sizeof(char *) * (i + 2));
	cmd = ft_strjoin("/", cmd);
	i = -1;
	while (paths[++i])
		full_paths[i] = ft_strjoin(paths[i], cmd);
	full_paths[i] = cmd;
	full_paths[i + 1] = NULL;
	return (full_paths);
}

void	run_bin(int num, t_info *info)
{
	char	**paths;
	int		i;

	paths = create_paths(info->commands[num].argv[0], info->envp);
	if (!paths)
		micro_print_err(info->commands[num].argv[0], 2);
	i = 0;
	while (paths[i])
	{
		if (!access(paths[i], X_OK))
			break ;
		i++;
		if (!paths[i])
			micro_print_err(info->commands[num].argv[0], 1);
	}
	execve(paths[i], info->commands[num].argv, info->envp);
}

int	check_cmd(char *cmd, int i, t_info *info)
{
	if (!ft_strncmp(cmd, "echo", ft_strlen("echo")))
		return (echo(info, i));
	if (!ft_strncmp(cmd, "cd", ft_strlen("cd")))
		return (cd(info, i));
	if (!ft_strncmp(cmd, "pwd", ft_strlen("pwd")))
		return (pwd(info, i));
	if (!ft_strncmp(cmd, "env", ft_strlen("pwd")))
		return (env(info, i));
	if (!ft_strncmp(cmd, "export", ft_strlen("export")))
		return (export(info, i));
	if (!ft_strncmp(cmd, "unset", ft_strlen("unset")))
		return (unset(info, i));
	if (!ft_strncmp(cmd, "exit", ft_strlen("exit")))
		return (ft_exit(info, i));
	return (0);
}

static void	redirects(t_exec *exec, t_info *info, int i)
{
	dup2(exec->fdin, 0);
	close(exec->fdin);
	if (i == info->cmd_num)
		exec->fdout = get_out_file(exec->tmpout, info);
	else
	{
		pipe(exec->fdpipe);
		exec->fdout = exec->fdpipe[1];
		exec->fdin = exec->fdpipe[0];
	}
	dup2(exec->fdout, 1);
	close(exec->fdout);
}

void	executor(t_info *info)
{
	int			i;
	t_exec		exec;

	exec.fdin = save_fd_set_input(info, &exec);
	i = -1;
	while (info->cmd_num >= ++i)
	{
		redirects(&exec, info, i);
		if (!check_cmd(info->commands->argv[0], i, info))
		{
			exec.pid = fork();
			if (exec.pid == 0)
				run_bin(i, info);
			waitpid(exec.pid, &exec.tmpret, 0);
			g_exit = WEXITSTATUS(exec.tmpret);
		}
	}
	restore_fd(&exec);
}
