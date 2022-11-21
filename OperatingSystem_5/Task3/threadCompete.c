#include <stdio.h>
#include <pthread.h>

void *thread_entry(void *param)
{
    int scope;
    pthread_attr_t attr;
    /* get the default attributes*/
    pthread_attr_init(&attr);
    /* first inquire on the current scope*/
    if (pthread_attr_getscope(&attr, &scope) != 0)
        fprintf(stderr, "Unable to get scheduling scope\n");
    else
    {
        if (scope == PTHREAD_SCOPE_PROCESS)
        {
            printf("PTHREAD_SCOPE_PROCESS\n");
            pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
            printf("Set to PTHREAD_SCOPE_SYSTEM\n");
            if (pthread_attr_getscope(&attr, &scope) != 0)
                fprintf(stderr, "Unable to get scheduling scope\n");
            else if (scope == PTHREAD_SCOPE_PROCESS)
                fprintf(stderr, "Set Failure\n");
            else if (scope == PTHREAD_SCOPE_SYSTEM)
                fprintf(stderr, "Set Success\n");
            else
                fprintf(stderr, "Illegal scope value.\n");
        }
        else if (scope == PTHREAD_SCOPE_SYSTEM)
        {
            printf("PTHREAD_SCOPE_SYSTEM\n");
            pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
            printf("Set to PTHREAD_SCOPE_PROCESS\n");
            if (pthread_attr_getscope(&attr, &scope) != 0)
                fprintf(stderr, "Unable to get scheduling scope\n");
            else if (scope == PTHREAD_SCOPE_SYSTEM)
                fprintf(stderr, "Set Failure\n");
            else if (scope == PTHREAD_SCOPE_PROCESS)
                fprintf(stderr, "Set Success\n");
            else
                fprintf(stderr, "Illegal scope value.\n");
        }
        else
            fprintf(stderr, "Illegal scope value.\n");
    }
}

int main(int argc, char *argv[])
{
    pthread_t tid;
    pthread_attr_t attr;
    printf("Set the default attr.\n");
    pthread_attr_init(&attr); // get default setting
    pthread_create(&tid, &attr, thread_entry, NULL);

    pthread_join(tid, NULL); // Wait for the thread to end.

    pthread_attr_init(&attr); // get default setting
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
    printf("\n\nSet the thread scope PTHREAD_SCOPE_PROCESS.\n");
    pthread_create(&tid, &attr, thread_entry, NULL);

    pthread_join(tid, NULL); // Wait for the thread to end.

    pthread_attr_init(&attr); // get default setting
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    printf("\n\nSet the thread scope PTHREAD_SCOPE_SYSTEM.\n");
    pthread_create(&tid, &attr, thread_entry, NULL);

    pthread_exit(0);
}
