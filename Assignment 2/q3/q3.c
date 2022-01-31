#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

const char *FILEPATH = "./file.txt";

/**
 * @brief write to fin, closes the fin
 *
 * @param fout the FILE* pointer to the file to be written
 */
void writeLineFILE(FILE *fout, pid_t p)
{

    fprintf(fout, "%d\n", p);
    fprintf(fout, "Hello\n");
    fprintf(fout, "My\n");
    fprintf(fout, "name\n");
    fprintf(fout, "is\n");
    fprintf(fout, "Abhiroop\n");
    fprintf(fout, "Mukherjee\n");
}

/**
 * @brief forks a process and read file from the child class only
 *
 * @param childNo user defiend childNo for printing puspose
 */
void forkAndWrite()
{
    FILE *fout = fopen(FILEPATH, "w"); // filepath created before fork
    pid_t p = fork();
    writeLineFILE(fout, p);
    fclose(fout);
}

int main()
{
    forkAndWrite();
    return 0;
}

// Observations:
// when one process is writing to the file, the other process is made to wait
// till the file writing is done by the previous process

/*
1. Can P2 write into the file abc using fp?
    A) Yes
2. If P2 can write into  abc then at which position in the file abc will it write?
    A) other process will be blocked till first process write is done, then other process will write
3. If P1 and P2 both attempt into the file abc then what will be written in the file?
    A) one will be blocked till other process write is done, then other will write
4. If P1 closes the file (fclose()) does it get closed for P2 as well?
    A) No
*/
