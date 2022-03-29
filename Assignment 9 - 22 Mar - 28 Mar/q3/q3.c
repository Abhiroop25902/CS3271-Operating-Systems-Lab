// hint: https://stackoverflow.com/questions/22194904/how-to-redirect-the-output-of-ls-or-any-other-shell-command-to-a-pipe
// hint: https://man7.org/linux/man-pages/man2/dup.2.html
// hint: https://stackoverflow.com/questions/12902627/the-difference-between-stdout-and-stdout-fileno
// hint: https://stackoverflow.com/questions/37603297/wc-standard-input-bad-file-descriptor-on-fork-pipe-execlp
// hint: https://stackoverflow.com/questions/8300301/chained-pipes-in-linux-using-pipe

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define ReadEnd 0
#define WriteEnd 1

void forkAndExecute(char *command, int pipeFDs[][2], int pipeFdSize, int readIdx, int writeIdx)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork fail");
    }

    if (pid == 0) // child
    {
        for (int i = 0; i < pipeFdSize; i++)
        {
            if ((i != readIdx) && (i != writeIdx)) // close all other pipes except readIdx and writeIdx
            {
                close(pipeFDs[i][ReadEnd]);
                close(pipeFDs[i][WriteEnd]);
            }
        }
        // why we closing other pipes also?
        // Ans: https://stackoverflow.com/questions/24766013/is-it-really-necessary-to-close-the-unused-end-of-the-pipe-in-a-process

        if (readIdx != -1) // -1 read supplied to the first command execve
        {
            dup2(pipeFDs[readIdx][ReadEnd], STDIN_FILENO); // set STDIN of this process to readEnd of read pipe
            close(pipeFDs[readIdx][WriteEnd]);             // close write end of the pipe we are reading from
        }

        if (writeIdx != -1) // -1 write supplied to the last command execve
        {
            dup2(pipeFDs[writeIdx][WriteEnd], STDOUT_FILENO); // set STDOUT of this process to writeEnd of write pipe
            close(pipeFDs[writeIdx][ReadEnd]);                // close readEnd of the pipe we are writing to
        }

        char *myArgv[] = {command, NULL};
        char *env[] = {NULL};
        if (execve(myArgv[0], myArgv, env) == -1)
        {
            perror("fork fail");
        }
    }
}

int main(int argc, char *argv[])
{
    int pipeFDs[argc - 2][2];

    for (int i = 0; i < argc - 2; i++)
    {
        if (pipe(pipeFDs[i]) < 0)
        {
            perror("pipe fail");
            exit(-1);
        }
    }

    forkAndExecute(argv[1], pipeFDs, argc - 2, -1, 0);

    for (int i = 2; i <= (argc - 2); i++)
    {
        forkAndExecute(argv[i], pipeFDs, argc - 2, i - 2, i - 1);
    }

    forkAndExecute(argv[argc - 1], pipeFDs, argc - 2, argc - 3, -1);

    wait(NULL); // wait for all child processes to complete
}