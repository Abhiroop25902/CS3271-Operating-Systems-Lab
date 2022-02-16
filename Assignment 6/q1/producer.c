/**
 * @file producer.c
 * @author Abhiroop Mukherjee (abhiroop.m25902@gmail.com) (510519109.abhirup@students.iiests.ac.in)
 * @brief Assignment 6 Question 1
 * @version 0.2
 * @date 2022-02-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>

#include <sys/types.h> //ftok(), semid()
#include <sys/ipc.h>   // ftok(), semid()
#include <sys/sem.h>   //semid()
#include <unistd.h>    //sleep()
#include <stdlib.h>    //exit()
#include <signal.h>    // for signal(), SIGKILL, kill()

#include <stdbool.h> // for true and false

const char *COMMON_FILE_PATH = "./common_file";
const int COMMON_PROJ_ID = 1;

const int SEM_SET_SIZE = 2; // one extra to keep count of the number of processes accessing the semaphore set

int semid = -1; // global for sighandler, made -1 to define not initialized 

typedef void (*sighandler_t)(int); // define the sighandler

/**
 * @brief wrapper for semop with error checking and reporting, semid required is made global
 * 
 * @param idx 
 * @param op_val the chage to be done to the semaphore of idx
 * @param sem_flg the sem_flg for the semop
 * @param verbose to add verbosity to the console
 */
void semOperation(int idx, int op_val, short sem_flg, bool verbose)
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
        exit(1);
    }
}

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
 * @brief Get the semaphore of given idx from the semaphore set coressponding to the semid (global variable)
 * 
 * @param idx 
 * @return int 
 */
int getSem(int idx)
{
    unsigned short int arr[SEM_SET_SIZE];
    union semun getAllArg;
    getAllArg.array = arr;
    int status = semctl(semid, 0, GETALL, getAllArg); // 0 doesn't matter

    // check if semctl was successful or not
    if (status == -1)
    {
        perror("semctl() failed");
        exit(1);
    }

    return arr[idx];
}

/**
 * @brief prints the semaphore set coressponding to the semid (global variable)
 *
 */
void print_sem_set()
{
    printf("sem set: ");
    unsigned short int arr[SEM_SET_SIZE];
    union semun getAllArg;
    getAllArg.array = arr;
    int status = semctl(semid, 0, GETALL, getAllArg);

    for (int i = 0; i < SEM_SET_SIZE; i++)
        printf("%d\t", arr[i]);
    printf("\n");
}

/**
 * @brief terminate the program by releasing the semaphore set coressponding to the semid
 *
 * @param signum
 */
void terminateProgram(int signum)
{
    printf("\n");

    semOperation(SEM_SET_SIZE - 1, -1, IPC_NOWAIT, true); //decrease the last semaphore by one to signify that this process will no longer use the semaphore
    print_sem_set();

    int status;
    if (getSem(SEM_SET_SIZE - 1) == 0) //the last semaphore will be zero iff no other processes are using it, if that's the case, we delete the semaphore
    {
        status = semctl(semid, 0, IPC_RMID);

        if (status == 0)
            fprintf(stderr, "Removed semaphore set with id = %d.\n", semid);
        else if (status == -1)
            fprintf(stderr, "Cannot remove semaphore set with id = %d.\n", semid);
        else
            fprintf(stderr, "semctl() returned wrong value while removing semaphore set with id = %d.\n", semid);
    }

    // now we have freed the semaphore set, now we kill the program itself
    status = kill(0, SIGKILL);
    // int kill(pid_t pid, int sig);
    // pid = 0 -> send sig all the processes in the group (also includes the child)
    // returns 0 if success, -1 on error

    if (status == 0)
        fprintf(stderr, "kill susccesful.\n"); /* this line may not be executed :P WHY?*/
    else if (status == -1)
        perror("kill failed.\n");
    else
        fprintf(stderr, "kill(2) returned wrong value.\n");
}

/**
 * @brief checks if the semaphore set corresponding to the semid is initialized or not
 * This is achived by using an extra semaphore og the semaphore set as a boolean,
 * if the last element is 1, it means that initialization is done, if it's 0,
 * it means initialization is pending
 *
 * @return true
 * @return false
 */
bool is_sem_set_initialized()
{
    return getSem(SEM_SET_SIZE - 1) != 0; //this will be 0 iff no process has used it before
}

int main()
{
    // set the signal handler to call terminateProgram() when SIGINT signal is given
    sighandler_t shandler = signal(SIGINT, terminateProgram);

    // get a unique key using the filepath and proj id
    key_t filekey = ftok(COMMON_FILE_PATH, COMMON_PROJ_ID);

    // use the common key to make/attack a semaphore set with SEM_SET_SIZE elements
    semid = semget(filekey, SEM_SET_SIZE, IPC_CREAT | 0666);
    if (semid == -1)
    {
        perror("semget failed ");
        exit(1);
    }
    else
        printf("Successfully got semaphore set with semid = %d\n", semid);

    if (!is_sem_set_initialized())
    {
        union semun setValArg;
        printf("Semaphore set not initialized, initializing it\n");
        setValArg.val = 1;
        semctl(semid, 0, SETVAL, setValArg); // set one in 0th index to set the value of semaphore for actual task
    }
    else
        printf("Semaphore set already initialized\n");

    semOperation(SEM_SET_SIZE - 1, +1, IPC_NOWAIT, true); // add tho the last element to say that this process is using the semaphore set

    print_sem_set();

    while (true)
    {
        semOperation(0, -1, SEM_UNDO, false);
        printf("I am producer!!\n");
        sleep(1);
        semOperation(0, +1, SEM_UNDO, false);
    }
}