/**
 * @file student.c
 * @author Abhiroop Mukherjee (abhiroop.m25902@gmail.com) (510519109.abhirup@students.iiests.ac.in)
 * @brief Assignment 7 q1 student
 * @version 0.1
 * @date 2022-02-22
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
#include <sys/sem.h> //semid(), sembuf, SEM_UNDO

const char *COUNT_FILEPATH = "./common_count";
const char *ROLL_FILEPATH = "./common_roll";
const char *SEM_FILEPATH = "./semaphore";
const int PROJ_ID = 1;

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
 * @brief wrapper for semop with error checking and reporting, semid required is made global
 *
 * @param idx
 * @param op_val the chage to be done to the semaphore of idx
 * @param sem_flg the sem_flg for the semop
 * @param verbose to add verbosity to the console
 */
void semOperation(int semid, int idx, int op_val, short sem_flg, bool verbose)
{
    struct sembuf op;

    /* struct sembuf has the following fields */
    // unsigned short sem_num;  -> semaphore number -> index of the semaphone in set
    // short          sem_op;   -> semaphore operation -> +ve number to add, -ve value try to decrement or wait,
    //                             0 will make process wait till semaphore is not 0
    // short          sem_flg;  -> operation flags
    op.sem_num = idx;
    op.sem_op = op_val;
    op.sem_flg = sem_flg; // SEM_UNDO -> opreation done to semaphore will be undone when the process completes
    // other option of flag -> IPC_NOWAIT -> throw error if process have to wait
    int status = semop(semid, &op, 1);

    if (status == 0)
    {
        if (verbose)
            printf("Succesfully did %d on index %d\n", op_val, idx);
    }
    else
    {
        perror("semop fail");
        exit(-1);
    }
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

    int roll = atoi(argv[1]);

    key_t sem_filekey = ftok(SEM_FILEPATH, PROJ_ID);
    int semid = semget(sem_filekey, 0, 0);
    semOperation(semid, 0, -1, SEM_UNDO, true);
    {
        key_t count_filekey = ftok(COUNT_FILEPATH, PROJ_ID);
        key_t roll_filekey = ftok(ROLL_FILEPATH, PROJ_ID);

        int countShmId = shmget(count_filekey, 0, 0);
        int RollShmId = shmget(roll_filekey, 0, 0);

        int *countPtr = (int *)getShmDataPtr(countShmId);
        int *rollPtr = (int *)getShmDataPtr(RollShmId);

        *countPtr = *countPtr + 1;
        *rollPtr = roll;

        shmdt(countPtr);
        shmdt(rollPtr);

        printf("Successfully Added Student %d to the attendance list\n", roll);
    }
    semOperation(semid, 0, +1, SEM_UNDO, true);

    return 0;
}