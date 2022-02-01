#include <stdio.h>
#include <unistd.h>    /* for fork() */
#include <sys/wait.h>  /* for wait() */
#include <sys/types.h> /* for wait() kill(2)*/
#include <sys/ipc.h>   /* for shmget() shmctl() */
#include <sys/shm.h>   /* for shmget() shmctl() */

#include <signal.h> /* for signal(2) kill(2) */

#include <errno.h> /* for perror */

#include <stdlib.h>

int shmid; /* defined globally so that signal handler can access it */

/* following is a signal handler for the keypress ^C, that is, ctrl-c */
typedef void (*sighandler_t)(int);
void releaseSHM(int signum)
{
    int status;
    // int shmctl(int shmid, int cmd, struct shmid_ds *buf);
    status = shmctl(shmid, IPC_RMID, NULL); /* IPC_RMID is the command for destroying the shared memory*/
    if (status == 0)
    {
        fprintf(stderr, "Remove shared memory with id = %d.\n", shmid);
    }
    else if (status == -1)
    {
        fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
    }
    else
    {
        fprintf(stderr, "shmctl() returned wrong value while removing shared memory with id = %d.\n", shmid);
    }

    // int kill(pid_t pid, int sig);
    status = kill(0, SIGKILL);
    if (status == 0)
    {
        fprintf(stderr, "kill susccesful.\n"); /* this line may not be executed :P WHY?*/
    }
    else if (status == -1)
    {
        perror("kill failed.\n");
        fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
    }
    else
    {
        fprintf(stderr, "kill(2) returned wrong value.\n");
    }
}

int main()
{
    int status;
    pid_t pid = 0;
    pid_t p1 = 0;

    sighandler_t shandler;

    /* install signal handler */
    // sighandler_t signal(int signum, sighandler_t handler);
    shandler = signal(SIGINT, releaseSHM); /* should we call this seperately in parent and child process */

    // int shmget(key_t key, size_t size, int shmflg);

    shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0777); // 0777 is octal here
    // absence of 0777 gives permission denied by shmat()
    //  File Permission in Linux
    //  1) me the user
    //  2) group
    //  3) root

    // file permission is la -la : drwxr-xr-x
    // 1st char -> file or directory
    // next 3 char -> read, write or execute for root
    // next 3 char -> read, write or execute for group
    // next 3 char -> read, write or execute for user

    if (shmid == -1)
    { /* shmget() failed() */
        perror("shmget() failed: ");
        exit(1);
    }

    printf("shmget() returns shmid = %d.\n", shmid);

    p1 = fork();
    if (p1 == 0)
    { /* child process */

        int i;
        int *pi_child;

        // void *shmat(int shmid, const void *shmaddr, int shmflg);
        pi_child = shmat(shmid, NULL, 0);
        if (pi_child == (void *)-1)
        { /* shmat fails */
            perror("shmat() failed at child: ");
            exit(1);
        }

        for (i = 0; i < 50; i++)
        {
            printf("Child Reads %d.\n", *pi_child);
            getchar();
        }
        exit(0);
    }
    else
    { /* parent process */
        int i;
        int *pi_parent;

        // void *shmat(int shmid, const void *shmaddr, int shmflg);
        pi_parent = shmat(shmid, NULL, 0);
        if (pi_parent == (void *)-1)
        { /* shmat fails */
            perror("shmat() failed at parent: ");
            exit(1);
        }

        for (i = 0; i < 50; i++)
        {
            *pi_parent = i;
            printf("Parent writes %d.\n", *pi_parent);
            getchar();
        }
        pid = wait(&status);
        printf("pid = %d status = %d!\n", pid, status);
        exit(0);
    }
}