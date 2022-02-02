/**
 * @file q2_type2.c
 * @author Abhiroop Mukherjee (510519109.abhirup@students.iiests.ac.in)
 * @date 2022-02-01
 *
 * @copyright Copyright (c) 2022
 *
 */

/*
Reresentation of Shared Memory Matrix:
int array in heap holding shmids of the 1D arrays, each shmid coressponds to row of matrix

Why? here for all process has been given thier own row, so no race condition is possible, no need to wait for one process to complete before starting other
*/

#include <stdio.h>
#include <sys/shm.h> // for shmget()
#include <sys/ipc.h> // for shmget()
#include <stdlib.h>  // for exit()
#include <unistd.h>  // for fork()
#include <wait.h>    //for waitpid()

int n = 2;
int m = 3;
int p = 4;
const int A[2][3] = {{1, 2, 3}, {4, 5, 6}};
const int B[3][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};

/**
 * @brief make 1D array in Shared Memory
 *
 * @param n size of the 1D arrays
 * @return int the shmid
 */
int makeShmArray(int p)
{
    int shmid = shmget(IPC_PRIVATE, sizeof(int) * p, IPC_CREAT | 0777);
    printf("Made SHM with id %d\n", shmid);
    if (shmid == -1)
    {
        printf("Error: can't make SHM");
        exit(0);
    }

    return shmid;
}

/**
 * @brief make an int array in heap corresponding to a row of matrix, each element contains shmid of 1D array coresspoing a row of matrix
 *
 * @param n number of rows
 * @param p number of columns
 * @return int*  adress of the int array in heap
 */
int *makeShmMatrix(int n, int p)
{
    int *shmidList = (int *)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++)
        shmidList[i] = makeShmArray(p);

    return shmidList;
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
 * @param i index for coressponding row of matrix
 * @param shmid the shmid coresspoing to the row of matrix
 */
void child_task(int i, int shmid)
{
    int *row_ptr = (int *)getShmDataPtr(shmid);

    if (row_ptr == (void *)-1)
    {
        perror("shmat fail");
        exit(1);
    }

    for (int j = 0; j < p; j++)
    {
        row_ptr[j] = 0;
        for (int k = 0; k < m; k++)
            row_ptr[j] += A[i][k] * B[k][j];
    }
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
 * @brief multiply two matrices A and B and store the result in shared memory matrix
 *
 * @param shmidList the list of shmid's coressponding to the rows of matrix
 */
void matrix_multiplication(int *shmidList)
{
    int pidList[n]; //pidList made to keep track of all the pids of child processess
    int pidIdx = 0;

    // Notice: all child processes are simultaneously run, only waiting is done by parent for all childs to complete.
    for (int i = 0; i < n; i++)
    {
        __pid_t pid = fork();
        if (pid == 0)
        {
            child_task(i, shmidList[i]);
            exit(0);
        }
        else
            pidList[pidIdx++] = pid;
    }

    // wait for all the child processess to complete matrix multiplication
    for (int i = 0; i < n; i++)
        wait_for_process(pidList[i]);
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
    for (int i = 0; i < p; i++)
        printf("%d\t", row_ptr[i]);
    shmdt(row_ptr);
}

/**
 * @brief print the matrix
 *
 * @param shmidList the list of shmid's coressponding to the rows of matrix
 */
void printMatrix(int *shmidList)
{
    for (int i = 0; i < n; i++)
    {
        printShmArray(shmidList[i]);
        printf("\n");
    }
}

/**
 * @brief frees the Shared Memory
 *
 */
void freeSHM(int shmid)
{
    int status = shmctl(shmid, IPC_RMID, NULL);
    if (status == 0)
        fprintf(stderr, "Remove shared memory with id = %d.\n", shmid);
    else if (status == -1)
        fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
    else
        fprintf(stderr, "shmctl() returned wrong value while removing shared memory with id = %d.\n", shmid);
}

/**
 * @brief frees all the shared memory rows
 *
 * @param shmidList list of shmid's coressponding to the rows of matrix
 */
void freeShmMatrix(int *shmidList)
{
    for (int i = 0; i < n; i++)
        freeSHM(shmidList[i]);
}

int main()
{
    int *shmidList = makeShmMatrix(n, p);
    matrix_multiplication(shmidList);

    printMatrix(shmidList);

    freeShmMatrix(shmidList);
    free(shmidList);

    return 0;
}