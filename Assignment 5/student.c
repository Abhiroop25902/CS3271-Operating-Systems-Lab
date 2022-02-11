/**
 * @file student.c
 * @author Abhiroop Mukherjee (abhiroop.m25902@gmail.com) (510519109.abhirup@students.iiests.ac.in)
 * @date 2022-02-08
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <stdio.h>
#include <time.h>    // for time()
#include <stdlib.h>  // for atoi(), exit()
#include <sys/shm.h> // for shmget(),  shmdt(), shmat()
#include <stdbool.h> // for bool
#include <string.h>  // for strlen()
#include <ctype.h>   // for isdigit()

const char *ARRAY_FILEPATH = "./common_array";
const char *ARRAY_SIZE_FILEPATH = "./common_array_size";
const int PROJ_ID = 1;

typedef struct att
{
    int roll;  // The roll number of the student giving the attendance
    time_t tm; // keep the time when attendance was given by the student.
    // time_t time(time_t *second) return time in sec from Jan 1, 1970;
    // if second is give, update it to currrent time also
} att;

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

    int idx = atoi(argv[1]);

    // get the common file shmkey to get the arrSize;
    int shmkey = ftok(ARRAY_SIZE_FILEPATH, PROJ_ID);
    int arrSizeShmid = shmget(shmkey, 0, 0);
    int *arrSizeShmPtr = (int *)getShmDataPtr(arrSizeShmid);
    int arrSize = *arrSizeShmPtr;
    shmdt(arrSizeShmPtr);

    // check if the given index is possible or not
    if (idx < 0 || idx >= arrSize)
    {
        printf("Error: index given in out of scope\n");
        exit(0);
    }

    // get common file shmkey to get/make same shared memory
    shmkey = ftok(ARRAY_FILEPATH, PROJ_ID);
    int arrShmid = shmget(shmkey, 0, 0);

    // set the roll number and current time
    att *attArrPtr = (att *)getShmDataPtr(arrShmid);
    if (attArrPtr[idx].roll == -1)
    {
        attArrPtr[idx].roll = idx;
        attArrPtr[idx].tm = time(NULL);
        printf("Successfully Added Student %d to the attendance list\n", idx);
    }
    else
        printf("Attendance already added, you should stop now!!");

    shmdt(attArrPtr);

    return 0;
}