/**
 * @file consumer.c
 * @author Abhiroop Mukherjee (abhiroop.m25902@gmail.com) (510519109.abhirup@students.iiests.ac.in)
 * @date 2022-02-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdio.h>

#include <sys/types.h> //ftok(), semid()
#include <sys/ipc.h>   // ftok(), semid()
#include <sys/sem.h>   //semid()
#include <unistd.h>    //sleep()

#include <stdbool.h> // for true and false

const char *COMMON_FILE_PATH = "./common_file";
const int COMMON_PROJ_ID = 1;

// for semctl
union semun
{
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO
                              (Linux-specific) */
};

/**
 * @brief does operation on semaphore of index idx of the semaphore set corressponding
 * to semid, operation is defined by op_val
 *
 * @param semid
 * @param idx
 * @param op_val
 */
void semOperation(int semid, int idx, int op_val)
{
    struct sembuf op;

    /* struct sembuf has the following fields */
    // unsigned short sem_num;  -> semaphore number -> index of the semaphone in set
    // short          sem_op;   -> semaphore operation -> +ve number to add, -ve value try to decrement or wait,
    //                             0 will make process wait till semaphore is not 0
    // short          sem_flg;  -> operation flags
    op.sem_num = idx;
    op.sem_op = op_val;
    op.sem_flg = SEM_UNDO; // SEM_UNDO -> opreation done to semaphore will be undone when the process completes
    // other option of flag -> IPC_NOWAIT -> throw error if process have to wait
    semop(semid, &op, 1);
}

int main()
{
    key_t filekey = ftok(COMMON_FILE_PATH, COMMON_PROJ_ID);

    int semid = semget(filekey, 1, IPC_CREAT | 0666);
    union semun setValArg;
    setValArg.val = 1;
    semctl(semid, 0, SETVAL, setValArg);

    while (true)
    {
        semOperation(semid, 0, -1);
        printf("I am consumer!!\n");
        sleep(1);
        semOperation(semid, 0, +1);
    }
}