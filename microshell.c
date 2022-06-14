#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


# define ERR_CD_ARG "error: cd: bad arguments\n"
# define ERR_CD_FATAL "error: cd: cannot change directory to"
# define ERR_FATAL "error: fatal\n"
# define ERR_CMD "error: cannot execute "

int ft_strlen(char *str)
{
    int i;

    i = 0;
    while(str[i])
        i++;
    return (i);
}

char *ft_strdup(char *str)
{
    char    *new;
    int     i;

    i = -1;
    new = malloc(1 * ft_strlen(str) + 1);
    while (str[++i])
        new[i] = str[i];
    new[i] = 0;
    return new;
}

void    ft_free_strs(char **strs)
{
    int i;

    i = 0;
    while (strs[i])
        free(strs[i++]);
    free(strs);
}

int put_error(char *msg, char *path)
{
    write(2, msg, ft_strlen(msg));
    if (path)
    {
        write(2, path, ft_strlen(path));
        write(2, "\n", 1);
    }
    return (1);
}

int ft_cd(char **arg)
{
    if (arg[2])
        return (put_error(ERR_CD_ARG, NULL));
    else if (chdir(arg[1]) == -1)
        return (put_error(ERR_CD_FATAL, arg[1]));
    return 0;
}


char    **get_word(char **arg, char *token)
{
    char    **pipeline;
    int     i;

    i = 0;
    while (arg[i] && strcmp(arg[i], token))
        i++;
    pipeline = malloc(8 * (i + 1));
    i = -1;
    while (arg[++i] && strcmp(arg[i], token))
        pipeline[i] = ft_strdup(arg[i]);
    pipeline[i] = 0;
    return (pipeline);
}

int next_token(char **arg, char *token)
{
    int     i;

    i = 0;
    while (arg[i] && strcmp(arg[i], token))
        i++;
    if (!arg[i])
        return (i);
    return (++i);
}

int pipex(char **arg, char **envp, int infile)
{
    int fd[2];
    int pid;

    if (pipe(fd) == -1)
        exit(put_error(ERR_FATAL, NULL));
    else if ((pid = fork()) == -1)
        exit(put_error(ERR_FATAL, NULL));
    if (!pid)
    {
        dup2(infile, STDIN_FILENO);
        dup2(fd[1], 1);
        close(infile);
        close(fd[1]);
        close(fd[0]);
        if (execve(arg[0], arg, envp) == -1)
            exit(put_error(ERR_CMD, arg[0]));
    }
    close(infile);
    close(fd[1]);
    return (fd[0]);
}

void    exec_last(char **arg, char **envp, int infile)
{
    int pid;

    if ((pid = fork()) == -1)
        exit(put_error(ERR_FATAL, NULL));
    if (!pid)
    {
        dup2(infile, STDIN_FILENO);
        close(infile);
        if (execve(arg[0], arg, envp) == -1)
            exit(put_error(ERR_CMD, arg[0]));
    }
    close(infile);
    while (waitpid(-1, NULL ,0) > 0)
        ;
}

void    exec_cmd(char **arg, char **envp)
{
    int fd_in;
    int i;
    char **cmd;

    i = 0;
    fd_in = dup(0);
    while (arg[i])
    {
        cmd = get_word(&arg[i], "|");
        i += next_token(&arg[i], "|");
        if (arg[i])
            fd_in = pipex(cmd, envp, fd_in);
        else
            exec_last(cmd, envp, fd_in);
        ft_free_strs(cmd);
    }
}

int main(int ac, char **av, char **envp)
{
    char    **pipeline;
    int i = 1;

    while (i < ac)
    {
        pipeline = get_word(&av[i], ";");
        if (!pipeline[0])
        {
            ft_free_strs(pipeline);
            continue ;
        }
        if (!strcmp(pipeline[0], "cd"))
            ft_cd(pipeline);
        else
            exec_cmd(pipeline, envp);
        i += next_token(&av[i], ";");
        ft_free_strs(pipeline);
    }
    // system("lsof -c a.out");
    return 0;
}
