#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

const char *FILENAME = "file.txt";

int main()
{
    FILE *fin = fopen(FILENAME, "r");

    while (!feof(fin))
    {
        if (fork() == 0)
        {
            sleep(1); // to make sure child acts first
            printf("child  => %c : %ld\n", fgetc(fin), ftell(fin));
            exit(0);
        }
        else
        {
            sleep(1); // to make sure child acts first
            printf("parent => %c : %ld\n", fgetc(fin), ftell(fin));
        }
    }

    return 0;
}

// int main()
// {

//     if (fork() == 0)
//     {
//         FILE *fin = fopen(FILENAME, "r");
//         while (!feof(fin))
//         {
//             sleep(1); // to make sure child acts first
//             printf("child  => %c : %ld\n", fgetc(fin), ftell(fin));
//         }
//         exit(0);
//     }
//     else
//     {
//         FILE *fin = fopen(FILENAME, "r");
//         while (!feof(fin))
//         {
//             sleep(1); // to make sure child acts first
//             printf("parent => %c : %ld\n", fgetc(fin), ftell(fin));
//         }
//     }

//     return 0;
// }