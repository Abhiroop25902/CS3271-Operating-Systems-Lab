/**
 * @file q1.c
 * @author Abhiroop Mukherjee (510519109.abhirup@students.iiests.ac.in)
 * @date 2022-02-01
 */

#include <stdio.h>
#include <stdlib.h> // for exit(), srand()
#include <string.h>
#include <unistd.h>  // for fork()
#include <sys/shm.h> // for shmget()
#include <sys/ipc.h> // for shmget()
#include <time.h>    // for time() for srand()

#include <signal.h> // for kill()

const char *filename = "./factorial";
const int MAX_FAC_INPUT = 21;

int shmid; // made this global for releaseSHM access when called by an inturrept signal;

typedef void (*sighandler_t)(int);

void releaseSHM(int signum)
{

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

long long factorial(int value)
{
    long long result = 1;

    for (int i = 2; i <= value; i++)
        result *= i;

    return result;
}

int *get_int_SHM_ptr()
{
    int *shm_int_pointer;
    shm_int_pointer = shmat(shmid, NULL, 0);
    // void *shmat(int shmid, const void *shmaddr, int shmflg);
    // - attaches the SHM corresponding to shmid if it exists
    // - if SHM exist, it returns the address of the SHM as void*""
    // - flags ain't used here but has it's uses -> "man shmat"
    // - On success, shmat() returns the address of the attached shared memory segment;
    //   on error, (void *) -1 is returned, and errno is set to indicate the cause
    //   of the error.

    if (shm_int_pointer == (void *)-1)
    {
        perror("shmat fail");
        exit(1);
    }

    return shm_int_pointer;
}

void childTask()
{
    int *shm_int_pointer = get_int_SHM_ptr();

    for (int i = 0; i < 50; i++)
    {
        printf("Factorial of %d = %lld", *shm_int_pointer, factorial(*shm_int_pointer));
        getchar();
    }

    exit(0);
}

void parentTask()
{
    srand(time(0));

    int *shm_int_pointer = get_int_SHM_ptr();

    for (int i = 0; i < 50; i++)
    {
        *shm_int_pointer = rand() % MAX_FAC_INPUT;
        printf("Parent updated SHM to %d", *shm_int_pointer);
        getchar();
    }
}

int main()
{
    // set program to call releaseSHM when inturrepted(SIGINT)
    sighandler_t shandler = signal(SIGINT, releaseSHM);

    shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0777);
    // int shmget(key_t key, size_t size, int shmflg);
    // creates a shared memory space and returns the shmid(unique identifier) of that memory space
    // IPC_PRIVATE and IPC_CREAT signals to create a SHM space of (size, but rounded up to page_size of OS)
    // can be used to get shmid of already defined SHM when key is not IPC_PRIVATE and shmflag as 0
    // return shmid of SHM or -1 if failed
    // 0777 is used to give permissions to the file

    if (shmid == -1)
    {
        printf("Error: can't make SHM");
        exit(0);
    }

    if (fork() == 0)
        childTask();
    else
        parentTask();

    return 0;
}