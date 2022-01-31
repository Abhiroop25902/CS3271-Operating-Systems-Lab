#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * @brief calculate the factorial of a given number n
 * 
 * NOTE: according to the way it is implemented, the output will be wrong for big numbers
 *
 * @param n the value for which factorial will be calculated
 * @return the factorial of n (long long)
 */
unsigned long long factorial(short n)
{
    unsigned long long result = 1;

    for (int i = 1; i <= n; i++)
        result *= i;

    return result;
}

/**
 * @brief forks a process to calculate the factoral and print the output and
 * also exits the child process to avoid recursions
 *
 * @param n the value for which factorial will be calculated
 */
void callProcessAndCalculateFactorial(int n)
{
    if (fork() == 0) // only process if the process is child process, else don't process
    {
        printf("factorial of %d = %lld\n", n, factorial(n));
        exit(0); // exit the child process
    }
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
        callProcessAndCalculateFactorial(atoi(argv[i]));
    return 0;
}