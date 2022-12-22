#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int segment_id = shmget(IPC_PRIVATE, 1024,
                            S_IRUSR | S_IWUSR);
    char *shared_memory = (char *)shmat(segment_id, NULL, 0);

    int pid = fork(); // Create a child process
    if (pid < 0)
    { // fork failed
        printf("Frok failure!\n");
    }
    else if (pid != 0)
    { // Father process
        // wait(NULL);
        printf("Parent Read: %s\n", shared_memory);
        usleep(10);
        // Detach and delete the shared memory.
        shmdt(shared_memory);
        shmctl(segment_id, IPC_RMID, NULL);
    }
    else if (pid == 0)
    { // Child process
        printf("Child Write: Hi There!\n");
        sprintf(shared_memory, "Hi There!");
    }
    return 0;
}
