#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int *a = malloc(sizeof(int));
    *a = 100;
    if (fork() == 0)
    {
        *a = 90;
        printf("child  => %p - %d\n", a, *a);
        exit(0);
    }
    else
    {
        sleep(0.01); // to make sure child acts first
        printf("parent => %p - %d\n", a, *a);
    }
    return 0;
}