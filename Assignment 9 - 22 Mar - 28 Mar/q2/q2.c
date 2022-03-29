// hint: https://stackoverflow.com/questions/22194904/how-to-redirect-the-output-of-ls-or-any-other-shell-command-to-a-pipe
// hint: https://man7.org/linux/man-pages/man2/dup.2.html
// hint: https://stackoverflow.com/questions/12902627/the-difference-between-stdout-and-stdout-fileno
// hint: https://stackoverflow.com/questions/37603297/wc-standard-input-bad-file-descriptor-on-fork-pipe-execlp

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define ReadEnd 0
#define WriteEnd 1

int main(int argc, char *argv[])
{

    int pipeFDs[2];
    if (pipe(pipeFDs) < 0)
    {
        perror("pipe fail");
        exit(-1);
    }
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork fail");
    }

    if (pid == 0) // child -> second command -> pipe read
    {
        dup2(pipeFDs[ReadEnd], STDIN_FILENO); // change input stream of this process with pipe
        close(pipeFDs[WriteEnd]);             // don't need child to write to the FD

        char *myArgv[] = {argv[2], NULL};
        if (execve(myArgv[0], myArgv, __environ) == -1)
        {
            perror("fork fail");
        }
    }
    else // parent -> first commnad -> pipe write
    {
        dup2(pipeFDs[WriteEnd], STDOUT_FILENO); // change ouptut stream of this process with pipe write
        close(pipeFDs[ReadEnd]);

        char *myArgv[] = {argv[1], NULL};
        if (execve(myArgv[0], myArgv, __environ) == -1)
        {
            perror("fork fail");
        }
    }
}