/**
 * @file q2.c
 * @author Abhiroop (510519109.abhirup@students.iiests.ac.in) (abhiroop.m25902@gmail.com)
 * @brief Assignment 3 Question 2
 * @date 2022-01-25
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_ARGV 100 // for defining max size of argv

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
    int idx = 1;
    while (idx < argc)
    {
        char *tempArgv[MAX_ARGV];
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

        forkAndExecute(tempArgv[0], tempArgv);
    }
    return 0;
}
