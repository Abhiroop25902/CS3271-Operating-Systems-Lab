// Write Function to support generic linked list with file handling

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> /* for thread functions */
#include <errno.h>   /* For the macros used here - EAGAIN, EINVAL, EPERM. etc.*/
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

//-----------------------LL------------------

typedef struct node // definition of node
{
    void *data;
    struct node *next;
    pthread_mutex_t node_mutex;

} node;

typedef struct gll // definition of gll
{
    int dsize;
    struct node *list;
    int (*compareFn)(void *a, void *b);
    void (*print_fn)(void *d);
    pthread_mutex_t gll_mutex;
} gll;

typedef struct gll *genericLL;

typedef struct passedData
{
    genericLL g;
    void *d;
} passedData;

void init_mutex(pthread_mutex_t mutex)
{
    int status = pthread_mutex_init(&mutex, NULL);

    if (status != 0)
    { /* pthread_mutex_init() failed */
        /* Do not use perror since pthreads functions  do  not  set  errno. */
        // perror("pthread_mutex_init() Failed: "); Cannot use this here

        /* Consult pthread_mutex_init() manual for information on return value for failure */
        char errorMessage[200];

        if (status == EAGAIN)
            strcpy(errorMessage, "The system lacked the necessary resources (other than memory) to initialize another mutex.");
        else if (status == ENOMEM)
            strcpy(errorMessage, "Insufficient memory exists to initialize the mutex.");
        else if (status == EPERM)
            strcpy(errorMessage, "The caller does not have the privilege to perform the operation.");
        else if (status == EINVAL)
            strcpy(errorMessage, "The attributes object referenced by attr has the robust mutex attribute set without the process-shared attribute being set.");
        else
            strcpy(errorMessage, "Unknown Error");

        /* Consult pthread_mutex_init() manual for information on return value for failure */
        fprintf(stderr, "pthread_mutex_init() failed: %s.\n", errorMessage);

        exit(1);
    }
}

void lock_mutex(pthread_mutex_t mutex)
{
    int status = pthread_mutex_lock(&mutex);

    if (status != 0)
    {
        /* Do not use perror since pthreads functions  do  not  set  errno. */
        // perror("pthread_mutex_init() Failed: "); Cannot use this here

        /* Consult pthread_mutex_init() manual for information on return value for failure */
        char errorMessage[200];

        if (status == EAGAIN)
            strcpy(errorMessage, "The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.");
        else if (status == EINVAL)
            strcpy(errorMessage, "The mutex was created with the protocol attribute having the value PTHREAD_PRIO_PROTECT and the calling thread's priority is higher than the mutex's current priority ceiling");
        else if (status == EPERM)
            strcpy(errorMessage, "The mutex was created with the protocol attribute having the value PTHREAD_PRIO_PROTECT and the calling thread is not in the real-time class (SCHED_RR or SCHED_FIFO scheduling class).");
        else
            strcpy(errorMessage, "Unknown Error");

        /* Consult pthread_mutex_init() manual for information on return value for failure */
        fprintf(stderr, "pthread_mutex_lock() failed: %s.\n", errorMessage);

        exit(1);
    }
}

void unlock_mutex(pthread_mutex_t mutex)
{
    int status = pthread_mutex_unlock(&mutex);

    if (status != 0)
    {
        /* Do not use perror since pthreads functions  do  not  set  errno. */
        // perror("pthread_mutex_init() Failed: "); Cannot use this here

        /* Consult pthread_mutex_init() manual for information on return value for failure */
        char errorMessage[200];

        if (status == EAGAIN)
            strcpy(errorMessage, "The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.");
        else if (status == EINVAL)
            strcpy(errorMessage, "The mutex was created with the protocol attribute having the value PTHREAD_PRIO_PROTECT and the calling thread's priority is higher than the mutex's current priority ceiling");
        else if (status == EPERM)
            strcpy(errorMessage, "The mutex was created with the protocol attribute having the value PTHREAD_PRIO_PROTECT and the calling thread is not in the real-time class (SCHED_RR or SCHED_FIFO scheduling class).");
        else
            strcpy(errorMessage, "Unknown Error");

        /* Consult pthread_mutex_init() manual for information on return value for failure */
        fprintf(stderr, "pthread_mutex_unlock() failed: %s.\n", errorMessage);

        exit(1);
    }
}

void thread_join(pthread_t threadId)
{
    int status = pthread_join(threadId, NULL);

    if (status != 0)
    {
        /* Do not use perror since pthreads functions  do  not  set  errno. */
        // perror("pthread_mutex_init() Failed: "); Cannot use this here

        /* Consult pthread_mutex_init() manual for information on return value for failure */
        char errorMessage[200];

        if (status == EDEADLK)
            strcpy(errorMessage, "A deadlock was detected (e.g., two threads tried to join with each other); or thread specifies the calling thread.");
        else if (status == EINVAL)
            strcpy(errorMessage, "thread is not a joinable thread OR Another thread is already waiting to join with this thread.");
        else
            strcpy(errorMessage, "Unknown Error");

        /* Consult pthread_mutex_init() manual for information on return value for failure */
        fprintf(stderr, "pthread_join() failed: %s.\n", errorMessage);

        exit(1);
    }
}

int int_compare(void *a, void *b) // compare function for int
{

    if (*(int *)a == *(int *)b)
        return 0;
    else if (*(int *)a < *(int *)b)
        return -1;
    else
        return 1;
}

void int_print(void *d)
{
    printf("%d", *((int *)d));
}

genericLL createGLL(int dsize, int (*cfunction)(void *, void *), void (*print_fun)(void *)) // makes Generic Linked List
{
    genericLL g;
    g = (gll *)malloc(sizeof(gll));

    g->dsize = dsize;
    g->list = NULL;
    g->compareFn = cfunction;
    g->print_fn = print_fun;
    init_mutex(g->gll_mutex);

    return g;
}

node *createNode(genericLL g, void *d)
{
    node *temp = (node *)malloc(sizeof(node));
    temp->data = malloc(g->dsize);
    memcpy(temp->data, d, g->dsize);
    temp->next = NULL;
    init_mutex(temp->node_mutex);
    return temp;
}

void *addNodeGLL(void *data) // add node in the end
{
    genericLL g = ((passedData *)data)->g;
    void *d = ((passedData *)data)->d;

    printf("adding ");
    g->print_fn(d);
    printf(" in linked list\n");

    node *temp = createNode(g, d);

    if (g->list == NULL)
    {
        lock_mutex(g->gll_mutex);
        g->list = temp;
        unlock_mutex(g->gll_mutex);
    }
    else
    {
        node *temp2 = g->list;

        while (temp2->next != NULL)
            temp2 = temp2->next;

        lock_mutex(temp2->node_mutex);
        temp2->next = temp;
        unlock_mutex(temp2->node_mutex);
    }
}

void *addNodeGLLSorted(void *data) // add node to make ascending order
{
    genericLL g = ((passedData *)data)->g;
    void *d = ((passedData *)data)->d;

    printf("adding ");
    g->print_fn(d);
    printf(" in linked list\n");

    node *temp = createNode(g, d);

    if (g->list == NULL)
    {
        lock_mutex(g->gll_mutex);
        g->list = temp;
        unlock_mutex(g->gll_mutex);
    }
    else
    {
        if (g->compareFn(temp->data, g->list->data) == -1) // curr val less than first node
        {
            temp->next = g->list;
            lock_mutex(g->gll_mutex);
            g->list = temp;
            unlock_mutex(g->gll_mutex);
        }
        else
        {
            node *current = g->list;
            node *previous = current;

            while (current != NULL && (g->compareFn(current->data, previous->data) == 1 || g->compareFn(current->data, previous->data) == 0)) // not null & >=
            {
                previous = current;
                current = current->next;
            }

            if (current == NULL)
            {
                lock_mutex(previous->node_mutex);
                previous->next = temp;
                unlock_mutex(previous->node_mutex);
            }
            else
            {
                temp->next = current;
                lock_mutex(previous->node_mutex);
                previous->next = temp;
                unlock_mutex(previous->node_mutex);
            }
        }
    }
}

bool isPresentGLL(genericLL g, void *d) // checks of value present in node
{
    node *temp = g->list;

    while (temp != NULL)
    {
        if (g->compareFn(temp->data, d) == 0)
            return true;

        temp = temp->next;
    }

    return false;
}

void *print_all(void *data) // prints all the value of the linked list of g as is
{
    genericLL g = (genericLL)data;

    printf("LIST : ");
    node *temp = g->list;

    while (temp != NULL)
    {
        g->print_fn(temp->data);
        printf("->");
        temp = temp->next;
    }

    printf("NULL\n");
}

void *deleteNodeGLL(void *data) // deletes node
{
    genericLL g = ((passedData *)data)->g;
    void *d = ((passedData *)data)->d;
    
    printf("Removing ");
    g->print_fn(d);
    printf(" from linked list\n");

    node *back = NULL;
    node *temp = g->list;

    while (temp != NULL)
    {
        if (g->compareFn(temp->data, d) == 0)
        {
            if (temp == g->list)
            {
                lock_mutex(g->gll_mutex);
                g->list = g->list->next;
                unlock_mutex(g->gll_mutex);
            }
            else
            {
                lock_mutex(back->node_mutex);
                back->next = temp->next;
                unlock_mutex(back->node_mutex);
            }

            lock_mutex(temp->node_mutex);
            free(temp);
            return NULL;
        }
        back = temp;
        temp = temp->next;
    }
}

int getNodeDataGLL(genericLL g, int i, void *d) // give output of ith node
{
    int pos = 1;
    node *temp = g->list;

    while (temp != NULL && pos <= i)
    {
        temp = temp->next;
        pos++;
    }
    if (temp == NULL)
        return 0;

    memcpy(d, temp->data, g->dsize);
    return 1;
}

void main()
{
    pthread_attr_t attr; /*Set of thread attributes required to be passed in pthread_create() */

    /* Set the default attributes to be used by pthread_create() */
    /* int pthread_attr_init(pthread_attr_t *attr); */
    pthread_attr_init(&attr); /* same attr will be used for creating all the threads */

    pthread_t ptids[15];

    genericLL g = createGLL(sizeof(int), int_compare, int_print); // makes stack

    int a = 1;
    passedData param1 = {g, &a};
    pthread_create(&ptids[0], &attr, addNodeGLL, &param1);
    pthread_create(&ptids[8], &attr, print_all, g);

    int c = 0;
    passedData param2 = {g, &c};
    pthread_create(&ptids[1], &attr, addNodeGLLSorted, &param2);
    pthread_create(&ptids[9], &attr, print_all, g);

    int b = 2;
    passedData param3 = {g, &b};
    pthread_create(&ptids[2], &attr, addNodeGLL, &param3);
    pthread_create(&ptids[3], &attr, print_all, g);

    int d = 5;
    passedData param4 = {g, &d};
    pthread_create(&ptids[4], &attr, addNodeGLLSorted, &param4);

    int e = 5;
    passedData param5 = {g, &e};
    pthread_create(&ptids[5], &attr, deleteNodeGLL, &param5);
    pthread_create(&ptids[6], &attr, print_all, g);

    pthread_create(&ptids[7], &attr, addNodeGLLSorted, &param5);

    pthread_create(&ptids[8], &attr, print_all, g);
    pthread_create(&ptids[9], &attr, print_all, g);
    pthread_create(&ptids[10], &attr, deleteNodeGLL, &param5);
    pthread_create(&ptids[11], &attr, print_all, g);
    pthread_create(&ptids[12], &attr, print_all, g);
    pthread_create(&ptids[13], &attr, addNodeGLL, &param3);
    pthread_create(&ptids[14], &attr, print_all, g);

    for (int i = 0; i < 15; i++)
        thread_join(ptids[i]);

    int f = 2;
    printf("isPresentNode for 2: %s\n", isPresentGLL(g, &f) ? "True" : "False");

    int *ptr = (int *)malloc(sizeof(int));
    getNodeDataGLL(g, 3, (void *)ptr);
    printf("getNodeDataGLL for 3: %d\n", *ptr);
}