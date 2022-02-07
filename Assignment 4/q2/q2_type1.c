/**
 * @file q2_type1.c
 * @author Abhiroop Mukherjee (510519109.abhirup@students.iiests.ac.in)
 * @date 2022-02-01
 *
 * @copyright Copyright (c) 2022
 *
 */

/*
Reresentation of Shared Memory Matrix:
n*p int shared memory array coresponding to array in row major scheme

Each childs can run simultaneosly provided each child writes do not collide with other children
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
 * @brief generates and returns shmid of shared memory of n*p size
 *
 * @param n
 * @param p
 * @return int
 */
int makeShmMatrix(int n, int p)
{
    int shmid = shmget(IPC_PRIVATE, sizeof(int) * n * p, IPC_CREAT | 0777);
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
 * @brief run the child task
 *
 * @param i index for coressponding row of matrix
 * @param shmid the shmid coresspoing to the row of matrix
 */
void child_task(int i, int shmid)
{
    int *matrix = (int *)getShmDataPtr(shmid);

    if (matrix == (void *)-1)
    {
        perror("shmat fail");
        exit(1);
    }

    for (int j = 0; j < p; j++)
    {
        matrix[i * n + j] = 0;
        for (int k = 0; k < m; k++)
            matrix[i * n + j] += A[i][k] * B[k][j];
    }
    shmdt(matrix);
}

/**
 * @brief multiply two matrices A and B and store the result in shared memory matrix
 *
 * @param shmidList the list of shmid's coressponding to the rows of matrix
 */
void matrix_multiplication(int shmid)
{
    int pidList[n]; // pidList made to keep track of all the pids of child processess
    int pidIdx = 0;

    // Notice: all child processes are simultaneously run, only waiting is done by parent for all childs to complete.
    // but here there is no race condition as all the childs writes in different location (if they write in same location, race condition will occur)
    for (int i = 0; i < n; i++)
    {
        __pid_t pid = fork();
        if (pid == 0)
        {
            child_task(i, shmid);
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
 * @brief print the matrix
 *
 * @param shmid shmid of the matrix stored as 1D array in row major order
 */
void printMatrix(int shmid)
{
    int *matrix = (int *)getShmDataPtr(shmid);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < p; j++)
            printf("%d\t", matrix[i * n + j]);

        printf("\n");
    }
}

/**
 * @brief frees the Shared Memory Matrix
 *
 * @param shmid
 */
void freeShmMatrix(int shmid)
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
    int shmid = makeShmMatrix(n, p);
    matrix_multiplication(shmid);

    printMatrix(shmid);

    freeShmMatrix(shmid);

    return 0;
}