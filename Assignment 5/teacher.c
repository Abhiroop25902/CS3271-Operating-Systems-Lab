/**
 * @file teacher.c
 * @author Abhiroop Mukherjee (abhiroop.m25902@gmail.com) (510519109.abhirup@students.iiests.ac.in)
 * @date 2022-02-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdio.h>
#include <time.h>    //for time()
#include <stdlib.h>  // for atoi(), exit(), qsort()
#include <sys/shm.h> // for shmget(), shmctl(), shmdt(), shmat()
#include <signal.h>  // for signal(), SIGKILL, kill()
#include <stdbool.h> // for bool
#include <string.h>  // for strlen()
#include <ctype.h>   // for isdigit()

// define common filepath and prject id for ftok
const char *COMMON_FILEPATH = "./common_file";
const int PROJ_ID = 1;

int shmid;   // made this global for sighandler
int arrSize; // made this global for sighandler

typedef void (*sighandler_t)(int); // define the sighandler

typedef struct att
{
    int roll;  // The roll number of the student giving the attendance
    time_t tm; // keep the time when attendance was given by the student.
    // time_t time(time_t *second) return time in sec from Jan 1, 1970;
    // if second is give, update it to currrent time also
} att;

/**
 * @brief comparator for the qsort
 *
 * @param att1
 * @param att2
 * @return int
 */
int attComparator(const void *att1, const void *att2)
{
    return (((att *)att1)->tm) - (((att *)att2)->tm);
}

/**
 * @brief Get the Shm Data Ptr object
 *
 * @param shmid
 * @return void* the data pointer of the shared memory
 */
void *getShmDataPtr(int shmid)
{
    void *data_ptr = shmat(shmid, NULL, 0);

    if (data_ptr == (void *)-1)
    {
        perror("shmat fail");
        exit(1);
    }

    return data_ptr;
}

/**
 * @brief prints the shared memeory attendace array in a chronological order
 *
 */
void printChronologicalOrder()
{
    att *attArrPtr = (att *)getShmDataPtr(shmid);

    qsort(attArrPtr, arrSize, sizeof(att), attComparator);

    for (int i = 0; i < arrSize; i++)
        if (attArrPtr[i].roll != -1)
            printf("Student %d attended\n", attArrPtr[i].roll);

    shmdt(attArrPtr);
}

/**
 * @brief terminate the program by printing attendace in chronologcal order,
 * closing all the shared memory, and then killing itself
 *
 * @param signum
 */
void terminateProgram(int signum)
{
    printf("\n");
    printChronologicalOrder(); // prints the attendace array in chronological order

    int status = shmctl(shmid, IPC_RMID, NULL);
    // int shmctl(int shmid, int cmd, struct shmid_ds *buf);
    // control the SHM held by shmid
    // what to do is defined by the cmd
    // here IPC_RMID is a signal to free the SHM
    // in ICP_RMID mode -> return 0 if success, -1 on error

    if (status == 0)
        fprintf(stderr, "Remove shared memory with id = %d.\n", shmid);
    else if (status == -1)
        fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
    else
        fprintf(stderr, "shmctl() returned wrong value while removing shared memory with id = %d.\n", shmid);

    // now we have freed the SHM, now we kill the program itself
    status = kill(0, SIGKILL);
    // int kill(pid_t pid, int sig);
    // pid = 0 -> send sig all the processes in the group (also includes the child)
    // returns 0 if success, -1 on error

    if (status == 0)
        fprintf(stderr, "kill susccesful.\n"); /* this line may not be executed :P WHY?*/
    else if (status == -1)
    {
        perror("kill failed.\n");
        fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
    }
    else
        fprintf(stderr, "kill(2) returned wrong value.\n");
}

/**
 * @brief Set the Shared Array object to default value
 *
 */
void setSharedArray()
{
    att *attArrPtr = (att *)getShmDataPtr(shmid);

    for (int i = 0; i < arrSize; i++)
        attArrPtr[i].roll = -1;

    shmdt(attArrPtr);
}

/**
 * @brief checks if the given string is all numbers or not
 * 
 * @param c 
 * @return true 
 * @return false 
 */
bool isCharNumeric(char *c)
{
    if (c == NULL)
        return false;
    for (int i = 0; i < strlen(c); i++)
    {
        if (!isdigit(c[i]))
            return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    // if parameters are not passed correctly
    if (argc != 2)
    {
        fprintf(stderr, "Error: Pass only 1 parameter\n");
        exit(1);
    }

    // if the parameter passed is not numeric
    if (!isCharNumeric(argv[1]))
    {
        fprintf(stderr, "Error: parameter passed is not number\n");
        exit(1);
    }

    arrSize = atoi(argv[1]);

    // if the parameter passed is -ve
    if (arrSize < 0)
    {
        fprintf(stderr, "Error: negative arr size is not allowed\n");
        exit(1);
    }

    // set program to call terminateProgram when inturrepted(SIGINT)
    sighandler_t shandler = signal(SIGINT, terminateProgram);

    // get common file shmkey to get/make same shared memory
    int shmkey = ftok(COMMON_FILEPATH, PROJ_ID);
    shmid = shmget(shmkey, sizeof(att) * arrSize, IPC_CREAT | 0666);

    setSharedArray(); // set default values;
    while (1)
    {
        // infintity loop to wait for the students
    }
}
