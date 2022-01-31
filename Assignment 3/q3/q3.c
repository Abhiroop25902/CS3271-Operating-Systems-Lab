/**
 * @file q3.c
 * @author Abhiroop (510519109.abhirup@students.iiests.ac.in) (abhiroop.m25902@gmail.com)
 * @brief Assignment 3 Question 3
 * @date 2022-01-25
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

/**
 * @brief forks a child process and executes given executable with argv
 *
 * @param filename the path of the executable
 * @param argv the argv to be passed
 * @return pid_t pid of the child process
 */
pid_t forkAndExecute(char *filename, char *argv[])
{
    pid_t pid = fork();
    if (pid == 0)
    {
        int status = execve(filename, argv, NULL);

        if (status == -1)
            perror("execve failed");

        exit(0);
    }

    return pid;
}

#define MAX_ARGV 100 // for defining max size of argv
#define MAX_PID 10   // for defining max size of pid array

int main(int argc, char *argv[])
{
    int idx = 1;
    pid_t childPids[MAX_PID];
    int childPidIdx = 0;
    while (idx < argc)
    {
        char *tempArgv[MAX_ARGV]; //argv array for only one command
        tempArgv[0] = argv[idx];
        int argvIdx = 1;
        idx++;

        while (idx < argc && argvIdx < MAX_ARGV && argv[idx][0] == '-')
        {
            tempArgv[argvIdx] = argv[idx];
            idx++;
            argvIdx++;
        }

        tempArgv[argvIdx] = NULL;

        childPids[childPidIdx] = forkAndExecute(tempArgv[0], tempArgv);
        childPidIdx++;
    }

    // wait for the child processes
    for (int i = 0; i < childPidIdx; i++)
    {
        int status;
        int result = waitpid(childPids[i], &status, 0);

        if (result == childPids[i])
            printf("Child %d successfully executed with status %d\n", childPids[i], status);
        else if (result == -1)
            printf("Child %d failed to execute\n", childPids[i]);
        else
            printf("Unknown Error of child %d, status = %d\n", childPids[i], status);
    }

    return 0;
}
