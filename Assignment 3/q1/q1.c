/**
 * @file q1.c
 * @author Abhiroop Mukherjee
 * @brief Assignment 3 Question 1
 * @date 2022-01-25
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * @brief forks a child process and executes given executable with argv
 *
 * @param filename the path of the executable
 * @param argv the argv to be passed
 */
void forkAndExecute(char *filename, char *argv[])
{
    pid_t pid = fork();
    if (pid == 0)
    {
        int status = execve(filename, argv, NULL);

        if (status == -1)
            perror("execve failed");

        exit(0);
    }
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        char *tempArgv[] = {argv[i], NULL};
        forkAndExecute(argv[i], tempArgv);
    }
    return 0;
}

// if we provide a text file instead of an executable,
// execve() fails are returns -1 and perror() prints "Exec format error"