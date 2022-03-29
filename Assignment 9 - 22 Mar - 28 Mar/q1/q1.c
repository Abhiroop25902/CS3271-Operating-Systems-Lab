#include <stdio.h>
#include <unistd.h>   /* read, write, pipe, _exit */
#include <stdlib.h>   /* exit functions */
#include <string.h>   //strlen
#include <sys/wait.h> /* wait */

#define ReadEnd 0
#define WriteEnd 1

void open_and_print(char *filename, int *pipeFDs)
{
    char data[1000];

    FILE *fin = fopen(filename, "r");
    while (fscanf(fin, "%[^\n] ", data) != EOF)       // the %[^\n] conversion specification, which matches a string of all characters not equal to the new line character ('\n') and stores it (plus a terminating '\0' character) in str.
        write(pipeFDs[WriteEnd], data, strlen(data)); /* write the bytes to the pipe */

    usleep(1000); // w8 for the original messege to be taken
    char endLine[] = "end";
    write(pipeFDs[WriteEnd], endLine, strlen(endLine));

    fclose(fin);
}

int find_num_words(char *readData, int size)
{
    // printf("%s", readData);
    int count = 0;

    for (int i = 0; i < size; i++)
        if (readData[i] == ' ')
            count++;

    // printf(" count: %d\n", count);
    return count == 0? 1: count;
}

int main(int argc, char **argv)
{
    int pipeFDs[2];
    if (pipe(pipeFDs) < 0)
    {
        perror("pipe fail");
        exit(-1);
    }

    pid_t cpid = fork();
    if (cpid < 0)
    {
        perror("fork fail");
        exit(-1);
    }

    if (cpid == 0) // child
    {
        char readData[1000];
        close(pipeFDs[WriteEnd]); /* child reads, doesn't write */
        int readChars = 0;

        int numChars = 0;
        int numWords = 0;
        int numLine = 0;

        while (readChars = read(pipeFDs[ReadEnd], readData, sizeof(readData)))
        {
            readData[readChars] = '\0';

            if (strcmp(readData, "end") == 0)
            {
                printf("Words: %d; Lines: %d; Chars: %d\n", numWords, numLine, numChars);
                numChars = 0;
                numWords = 0;
                numLine = 0;
            }
            else
            {
                numChars += strlen(readData);
                numLine++;
                numWords += find_num_words(readData, strlen(readData));
            }
            // printf("%s\n", readData);
        }
        // write(STDOUT_FILENO, &readData, strlen(readData));

        close(pipeFDs[ReadEnd]); /* close the ReadEnd: all done */
        _exit(0);
    }
    else
    {                            // parent
        close(pipeFDs[ReadEnd]); /* parent writes, doesn't read */

        for (int i = 1; i < argc; i++)
            open_and_print(argv[i], pipeFDs);

        close(pipeFDs[WriteEnd]); /* done writing: generate eof */

        wait(NULL); /* wait for child to exit */
        exit(0);
    }

    return 0;
}