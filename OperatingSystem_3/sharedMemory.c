#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    int segment_id;
    char *shared_memory;
    const int size = 4096;

    segment_id = shmget(IPC_PRIVATE, size, S_IRUSR | S_IWUSR);
    shared_memory = (char*)shmat(segment_id, NULL, 0);
    int pid = fork(); // Create a child process
    if(pid < 0)
    {
        // fork failed
        printf("Frok failure!\n");
    }
    else if(pid != 0)
    { // Father process
        // wait(NULL);
        printf("Parent Read: %s\n", shared_memory);
        shmdt(shared_memory);
        shmctl(segment_id, IPC_RMID, NULL);
    }
    else if(pid == 0)
    { // Child process
        printf("Child Write: Hi There!\n");
        sprintf(shared_memory, "Hi There!");
    }
    return 0;
}
