#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int status;
    char *myargv[] = {"/bin/ls", "-l", NULL};
    status = execve("/bin/ls", myargv, NULL);
    if (status == -1)
        perror("Exec Fails: ");

}