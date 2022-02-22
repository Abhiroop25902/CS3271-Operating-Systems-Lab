/**
 * @file teacher.c
 * @author Abhiroop Mukherjee (abhiroop.m25902@gmail.com) (510519109.abhirup@students.iiests.ac.in)
 * @brief Assignment 7 q1 teacher
 * @version 0.1
 * @date 2022-02-22
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdio.h>
#include <time.h>    //for time()
#include <stdlib.h>  // for atoi(), exit()
#include <sys/shm.h> // for shmget(), shmctl(), shmdt(), shmat()
#include <signal.h>  // for signal(), SIGKILL, kill()
#include <stdbool.h> // for bool
#include <string.h>  // for strlen()
#include <ctype.h>   // for isdigit()
#include <sys/sem.h> //semid(), sembuf, SEM_UNDO

// define common filepath and prject id for ftok
const char *COUNT_FILEPATH = "./common_count";
const char *ROLL_FILEPATH = "./common_roll";
const char *SEM_FILEPATH = "./semaphore";
const int PROJ_ID = 1;
const int SEM_SET_SIZE = 1;

int countShmid; // made this global for sighandler
int rollShmid;  // made this global for sighandler
int semid;      // made this global for sighandler

typedef struct att
{
    int roll;  // The roll number of the student giving the attendance
    time_t tm; // keep the time when attendance was given by the student.
    // time_t time(time_t *second) return time in sec from Jan 1, 1970;
    // if second is give, update it to currrent time also
} att;

att *attendanceArray;     // made this global for sighandler
int attendanceArrMaxSize; // made this global for sighandler

typedef void (*sighandler_t)(int); // define the sighandler

/**
 * @brief Create a File object with given filepath
 *
 * @param filepath
 */
void createFile(const char *filepath)
{
    // get common file shmkey to get/make same shared memory
    printf("Generating a common file \"%s\" for inter process communication\n", filepath);
    FILE *fp = fopen(filepath, "w");
    if (fp == NULL)
    {
        perror("File not able to made");
        exit(0);
    }
    fclose(fp);
}

/**
 * @brief delete file given
 *
 * @param filepath path of the file to be deleted
 */
void deleteFile(const char *filepath)
{
    printf("Attempting to delete temp file \"%s\"\n", filepath);
    int removeStatus = remove(filepath);

    if (removeStatus == 0)
        printf("Syccessfully removed %s\n", filepath);
    else if (removeStatus == -1)
        perror("failed to delete file");
}

void allocateSemSet(const char *filepath, int *semidPtr, int semSetSize)
{
    createFile(filepath);
    key_t filekey = ftok(filepath, PROJ_ID);
    *semidPtr = semget(filekey, semSetSize, IPC_CREAT | 0666);

    if (semid == -1)
    {
        perror("semget failed ");
        exit(-1);
    }
    else
        printf("Successfully got semaphore set with semid = %d\n", semid);
}

/**
 * @brief set val in idx in sem set coressponding to given semid
 *
 * @param semid
 * @param idx
 * @param val
 */
void semSetVal(int semid, int idx, int val)
{
    // for semctl
    union semun
    {
        int val;               /* Value for SETVAL */
        struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
        unsigned short *array; /* Array for GETALL, SETALL */
        struct seminfo *__buf; /* Buffer for IPC_INFO
                                  (Linux-specific) */
    };

    union semun setValArg;
    setValArg.val = val;
    semctl(semid, idx, SETVAL, setValArg);
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
 * @brief deallocate the semaphore set coressponding to semid
 *
 * @param semid
 */
void deallocateSemSet(int semid)
{
    int status = semctl(semid, 0, IPC_RMID);

    if (status == 0)
        fprintf(stderr, "Removed semaphore set with id = %d.\n", semid);
    else if (status == -1)
        fprintf(stderr, "Cannot remove semaphore set with id = %d.\n", semid);
    else
        fprintf(stderr, "semctl() returned wrong value while removing semaphore set with id = %d.\n", semid);
}

/**
 * @brief create a shared memory of given size coressponding to filepath, and returns generated shmid in shmid
 *
 * @param filepath
 * @param shmid
 * @param size
 */
void createSHM(const char *filepath, int *shmid, size_t size)
{
    // get common file shmkey to get/make same shared memory
    createFile(filepath);
    int shmkey = ftok(filepath, PROJ_ID);
    *shmid = shmget(shmkey, size, IPC_CREAT | 0666);
    if (*shmid == -1)
    {
        perror("shmget fail");
        exit(-1);
    }
    printf("Successfully created shared memory of size %ld with shmid = %d\n", size, *shmid);
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
        exit(-1);
    }

    return data_ptr;
}

/**
 * @brief deallocates the shared memory coressponding to the given shmid
 *
 * @param shmid
 */
void deallocateShm(int shmid)
{
    int status = shmctl(shmid, IPC_RMID, NULL);
    // int shmctl(int arrShmid, int cmd, struct shmid_ds *buf);
    // control the SHM held by arrShmid
    // what to do is defined by the cmd
    // here IPC_RMID is a signal to free the SHM
    // in ICP_RMID mode -> return 0 if success, -1 on error

    if (status == 0)
        fprintf(stderr, "Remove shared memory with id = %d.\n", shmid);
    else if (status == -1)
        fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
    else
        fprintf(stderr, "shmctl() returned wrong value while removing shared memory with id = %d.\n", shmid);
}

/**
 * @brief prints the shared memeory attendace array in a chronological order
 *
 */
void printChronologicalOrder()
{
    // semOperation(semid, 0, -1, SEM_UNDO, false);
    int *countPtr = (int *)getShmDataPtr(countShmid);
    int currSize = *countPtr;
    shmdt(countPtr);
    // semOperation(semid, 0, +1, SEM_UNDO, false);

    for (int i = 0; i <= currSize; i++)
        printf("Student %d attended; Time: %s", attendanceArray[i].roll, ctime(&(attendanceArray[i].tm)));
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

    deallocateShm(countShmid);
    deallocateShm(rollShmid);
    deallocateSemSet(semid);

    deleteFile(COUNT_FILEPATH);
    deleteFile(ROLL_FILEPATH);
    deleteFile(SEM_FILEPATH);

    free(attendanceArray);

    // now we have freed the SHM, now we kill the program itself
    int status = kill(0, SIGKILL);
    // int kill(pid_t pid, int sig);
    // pid = 0 -> send sig all the processes in the group (also includes the child)
    // returns 0 if success, -1 on error

    if (status == 0)
        fprintf(stderr, "kill susccesful.\n"); /* this line may not be executed :P WHY?*/
    else if (status == -1)
        perror("kill failed.\n");
    else
        fprintf(stderr, "kill(2) returned wrong value.\n");

    exit(-1);
}

/**
 * @brief checks if the given string is all numbers or not
 *
 * @param c
 * @return true when the string has all numeric characters
 * @return false otherwise
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
        exit(-1);
    }

    // if the parameter passed is not numeric
    if (!isCharNumeric(argv[1]))
    {
        fprintf(stderr, "Error: parameter passed is not number\n");
        exit(-1);
    }

    attendanceArrMaxSize = atoi(argv[1]);

    // if the parameter passed is -ve
    if (attendanceArrMaxSize < 0)
    {
        fprintf(stderr, "Error: negative arr size is not allowed\n");
        exit(-1);
    }
    attendanceArray = malloc(sizeof(att) * attendanceArrMaxSize);

    // set program to call terminateProgram when inturrepted(SIGINT)
    sighandler_t shandler = signal(SIGINT, terminateProgram);

    createSHM(COUNT_FILEPATH, &countShmid, sizeof(int));
    createSHM(ROLL_FILEPATH, &rollShmid, sizeof(int));

    int *countPtr = (int *)getShmDataPtr(countShmid);
    *countPtr = -1;
    shmdt(countPtr);

    int *rollPtr = (int *)getShmDataPtr(rollShmid);
    *rollPtr = -1;
    shmdt(rollPtr);

    allocateSemSet(SEM_FILEPATH, &semid, 1);
    semSetVal(semid, 0, 1);

    bool exit = false;
    while (1)
    {
        semOperation(semid, 0, -1, SEM_UNDO, false);
        rollPtr = (int *)getShmDataPtr(rollShmid);
        if (*rollPtr != -1)
        {
            countPtr = (int *)getShmDataPtr(countShmid);

            if (*rollPtr >= attendanceArrMaxSize)
            {
                printf("Malicious Person found sending out of bound roll %d", *rollPtr);
                *countPtr = *countPtr - 1;
            }
            else
            {
                attendanceArray[*countPtr].roll = *rollPtr;
                attendanceArray[*countPtr].tm = time(NULL);
            }

            *rollPtr = -1;

            if (*countPtr == attendanceArrMaxSize - 1)
            {
                printf("Maximum Limit Reached, exiting\n");
                exit = true;
            }

            shmdt(countPtr);
        }
        shmdt(rollPtr);
        semOperation(semid, 0, +1, SEM_UNDO, false);

        if (exit)
            terminateProgram(SIGINT);
    }
}
