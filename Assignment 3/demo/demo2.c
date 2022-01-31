#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int status;
    pid_t pid = 0;
    pid_t p1 = 0;
    printf("Hello World!.\n");

    p1 = fork();
    if (p1 == 0)
    {
        int i;
        for (i = 0; i < 5; i++)
        {
            printf("%d\n", i++);
            // getchar();
        }
        exit(0);
    }
    pid = wait(&status);
    printf("pid = %d status = %d!\n", pid, status);
}
