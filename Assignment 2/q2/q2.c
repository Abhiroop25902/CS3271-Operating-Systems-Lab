#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

const char *FILEPATH = "./file.txt";

/**
 * @brief read from fin, closes the fin and returns the first line
 *
 * @param fin
 * @return char*
 */
void readLineFILE(FILE *fin, bool isChild)
{

    size_t maxChar = 100;

    char *process = isChild ? "child" : "parent";

    char c;
    printf("%s started\n", process);
    while (((c = getc(fin)) != EOF))
    {

        // char c = fgetc(fin);
        if (isalnum(c) || c == ' ')
            printf("%s => %c, %d\n", process, c, c);
        else
        {
            printf("error from %s\n", process);
            exit(0);
        }

        sleep(1);
    }
}

/**
 * @brief forks a process and read file from the child class only
 *
 */
void forkAndRead()
{
    FILE *fin = fopen(FILEPATH, "r"); // filepath created before fork
    pid_t p = fork();

    if (p == 0)
    {
        readLineFILE(fin, true);
        exit(0);
    }
    else
        readLineFILE(fin, false);

    fclose(fin);
}

int main()
{
    forkAndRead();
    return 0;
}
// Observations
//  while one is reading, other one cannot read (even if we do character by charecter)
// NOTE: if we open file after fork, it will be read by both the child and parent as expected
// but if we open file before fork, it will be read by only one of the child and parent process

/*
Case 1: Fork -> open file -> close file -> exit child
1. Can P2 read from the file abc using fp?
    A) Yes
2. If P2 can read from abc then from which position in the file abc will it read?
    A) From the beginning of the file
3. If P1 and P2 both attempt to read from abc then what will they read?
    A) both read all the data
4. If P1 closes the file (fclose()) does it get closed for P2 as well?
    A) No
*/

/*
Case 2: open file -> Fork -> exit child -> close file
1. Can P2 read from the file abc using fp?
    A) Yes
2. If P2 can read from abc then from which position in the file abc will it read?
    A) can't access at all
3. If P1 and P2 both attempt to read from abc then what will they read?
    A) only one reads
4. If P1 closes the file (fclose()) does it get closed for P2 as well?
    A) doesn't matter
*/
