#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

const char *FILENAME = "file.txt";
const char* SENTENCE = "Hello";
const char* SENTENCE2 = "World";

int main()
{
    FILE *fin = fopen(FILENAME, "w");

    for(int i = 0; i < strlen(SENTENCE); i++)
    {
        if (fork() == 0)
        {
            sleep(1); // to make sure child acts first
            printf("child  => %c : %ld\n", fprintf(fin,"%c", SENTENCE[i]), ftell(fin));
            exit(0);
        }
        else
        {
            sleep(1); // to make sure child acts first
            printf("parent => %c : %ld\n", fprintf(fin,"%c", SENTENCE[i]), ftell(fin));
        }
    }

    return 0;
}

// int main()
// {

//     if (fork() == 0)
//     {
//         FILE *fin = fopen(FILENAME, "w");
//         for(int i = 0; i < strlen(SENTENCE); i++)
//         {
//             // sleep(1); // to make sure child acts first
//             printf("child  => %c : %ld\n", fprintf(fin,"%c", SENTENCE[i]), ftell(fin));
//         }
//         exit(0);
//     }
//     else
//     {
//         FILE *fin = fopen(FILENAME, "w");
//         for(int i = 0; i < strlen(SENTENCE2); i++)
//         {
//             // sleep(1); // to make sure child acts first
//             printf("parent => %c : %ld\n", fprintf(fin,"%c", SENTENCE2[i]), ftell(fin));
//         }
//     }

//     return 0;
// }