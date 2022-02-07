/**
 * @file q2_practice.c
 * @author Abhiroop Mukherjee (510519109.abhirup@students.iiests.ac.in)
 * @date 2022-02-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/*
Practice for q2 -> working with 1D arrays

*/

#include <stdio.h>
#include <sys/shm.h> // for shmget()
#include <sys/ipc.h> // for shmget()
#include <stdlib.h>  // for exit()
#include <unistd.h>  // for fork()
#include <wait.h>

int n = 10;

/**
 * @brief make 1D array in Shared Memory
 * 
 * @param n size if the 1D arrays
 * @return int the shmid
 */
int makeShmArray(int n)
{
    int shmid = shmget(IPC_PRIVATE, sizeof(int) * n, IPC_CREAT | 0777);
    printf("Made SHM with id %d\n", shmid);
    if (shmid == -1)
    {
        printf("Error: can't make SHM");
        exit(0);
    }

    return shmid;
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
 * @brief run the child task
 * 
 * @param i index for Shared Memory Array
 * @param shmid 
 */
void child_task(int i, int shmid)
{
    int *row_ptr = (int *)getShmDataPtr(shmid);

    if (row_ptr == (void *)-1)
    {
        perror("shmat fail");
        exit(1);
    }

    row_ptr[i] = i;
    shmdt(row_ptr);
}

/**
 * @brief wait for the process with given pid and provide diagnostic data
 * 
 * @param pid the process id to wait for
 */
void wait_for_process(int pid)
{
    int status;
    int result = waitpid(pid, &status, 0);

    if (result == pid)
        printf("Child %d successfully executed with status %d\n", pid, status);
    else if (result == -1)
        printf("Child %d failed to execute\n", pid);
    else
        printf("Unknown Error of child %d, status = %d\n", pid, status);
}

/**
 * @brief update the Share Memory connected to shmid via child processes
 * 
 * @param shmid 
 */
void array_update(int shmid)
{
    for (int i = 0; i < n; i++)
    {
        __pid_t pid = fork();
        if (pid == 0)
        {
            child_task(i, shmid);
            exit(0);
        }
        else
            wait_for_process(pid);
    }
}

/**
 * @brief print the shared memory array
 * 
 * @return  
 */
void printShmArray(int shmid)
{
    int *row_ptr = (int *)getShmDataPtr(shmid);
    if (row_ptr == (void *)-1)
    {
        perror("shmat fail");
        exit(1);
    }
    for (int i = 0; i < n; i++)
        printf("%d ", row_ptr[i]);
    shmdt(row_ptr);
}

/**
 * @brief frees the Shared Memory
 * 
 */
void freeSHM(shmid)
{
    int status = shmctl(shmid, IPC_RMID, NULL);
    if (status == 0)
        fprintf(stderr, "Remove shared memory with id = %d.\n", shmid);
    else if (status == -1)
        fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
    else
        fprintf(stderr, "shmctl() returned wrong value while removing shared memory with id = %d.\n", shmid);
}

int main()
{
    int shmid = makeShmArray(n);
    array_update(shmid);

    printShmArray(shmid);

    freeSHM(shmid);

    return 0;
}