#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork Failed");
        exit(-1);
    }
    else if (pid == 0)
    {
        printf("I am a child\n");
    }
    else
    {
        // wait(NULL);
        printf("I am a parent\n");
        exit(0);
    }
}
