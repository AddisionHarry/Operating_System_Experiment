#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <sys/wait.h>

// Remember to use 'gcc -o sem sem_1.c -pthread -lm' when compiling.

// #define USING_FILE_OPERATION

#define KEY_SHARED_MEMORY 120
#define BUFFER_NUM_MAX 10
#define WRITE_NUM_MAX 6

typedef struct
{
    bool GetMaxNumber; // The flag that represent the consumer has consume all the numbers.
    void *bufferPos;   // The buffer used among threads.
    sem_t *Mutex;      // The sem lock used for lock among threads.
    sem_t *full;       // the sem represent that the buffer is full.
} SharedData_t;        // The data shared among the threads.

/**
 * @brief Write a number to the specific pos in the file.
 * @author fwlh
 * @param  buffer           the buffer to be operated.
 * @param  pos              which pos to be written.
 * @param  Number           the number to write.
 */
static void Write_Number_IntoBuff(void *buffer, int pos, int Number)
{
#ifdef USING_FILE_OPERATION
    /* Check the input params. */
    assert(pos < BUFFER_NUM_MAX);
    assert(pos >= 0);
    assert(Number >= 0);
    assert(Number <= WRITE_NUM_MAX);
    assert(file);
    /* Write the number into the file. */
    printf("fseek return :%d\n", fseek(file, 5 * pos, SEEK_SET));
    fprintf(file, "%4d", Number);
#else
    int *bufferMemory = (int *)buffer;
    bufferMemory[pos] = Number;
#endif
}

/**
 * @brief Write the inited bank into the buffer.
 * @author fwlh
 * @param  buffer           the buffer to be operated.
 * @param  pos              which pos to be written.
 */
static void Write_Blank_IntoBuff(void *buffer, int pos)
{
#ifdef USING_FILE_OPERATION
    /* Check the input params. */
    assert(pos < BUFFER_NUM_MAX);
    assert(pos >= 0);
    assert(file);
    /* Write the number into the file. */
    fseek(file, 5 * pos, SEEK_SET);
    for (int i = 0; i < 4; ++i)
        fputc(' ', file);
    fputc(',', file);
#else
    int *bufferMemory = (int *)buffer;
    bufferMemory[pos] = -1;
#endif
}

/**
 * @brief Read a number from the specific pos in the file.
 * @author fwlh
 * @param  buffer           the buffer to be operated.
 * @param  pos              which pos to read.
 * @param  Number           the number read back from.
 * @return int              the number read back from.
 */
static int Read_Number_FromBuff(void *buffer, int pos, int *Number)
{
#ifdef USING_FILE_OPERATION
    /* Check the input params. */
    assert(pos < BUFFER_NUM_MAX);
    assert(pos >= 0);
    assert(file);

    /* Read the specific number. */
    int numberRead = 0;
    fseek(file, 5 * pos, SEEK_SET);
    fscanf(file, "%4d", &numberRead);

    /* Clear the number on the corresponding pos and return. */
    Write_Blank_IntoBuff(file, pos);
    if (Number)
        *Number = numberRead;
    return numberRead;
#else
    int *bufferMemory = (int *)buffer;
    int numberRead = bufferMemory[pos];
    if (Number)
        *Number = numberRead;
    Write_Blank_IntoBuff(buffer, pos);
    return numberRead;
#endif
}

void *Producer_Entry(SharedData_t *sharedMemory)
{
    int writePos = 0, WriteNumber = 0;
    while (WriteNumber <= WRITE_NUM_MAX)
    {
        /* Inquire the authority to write&read. */
        sem_wait(sharedMemory->Mutex);
        /* Get the authority to write&read, then write&read data. */
        // First judge whether the buffer is full now.
        if (Read_Number_FromBuff(sharedMemory->bufferPos, BUFFER_NUM_MAX - 1, &writePos) != BUFFER_NUM_MAX - 1)
        {
            // Write the corresponding number to the right position.
            Write_Number_IntoBuff(sharedMemory->bufferPos, writePos, WriteNumber);
            // Update the new valid position.
            Write_Number_IntoBuff(sharedMemory->bufferPos, BUFFER_NUM_MAX - 1, writePos + 1);
            // Increase the number to write.
            WriteNumber++;
        }
        else
        {
            // Now the buffer is full, thus nothing to operate but not write the number.
            Write_Number_IntoBuff(sharedMemory->bufferPos, BUFFER_NUM_MAX - 1, writePos);
        }
        /* Return the authority to write. */
        sem_post(sharedMemory->Mutex);
    }
    for (int i = 0; i < BUFFER_NUM_MAX; ++i)
        printf("%d : %d\n", i, ((int *)sharedMemory->bufferPos)[i]);
    printf("\n\n\n\n Producer End! \n\n\n\n\n");
    return NULL;
}

void *Consumer_Entry(void)
{
    /* Create shared memory. */
    int shmid = shmget((key_t)KEY_SHARED_MEMORY, sizeof(SharedData_t), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }
    void *shm = shmat(shmid, (void *)0, 0);
    if (shm == (void *)-1)
    {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    SharedData_t *sharedMemory = (SharedData_t *)shm;

    int WritePos = 1, ReadData = 0;
    struct timespec timeout = {
        .tv_nsec = 0,
        .tv_sec = 1,
    };

    while ((!sharedMemory->GetMaxNumber) || (WritePos > 0))
    {
        /* Inquire the authority to write&read. */
        for (int i = 0; i < BUFFER_NUM_MAX; ++i)
            printf("%d : %d\n", i, ((int *)sharedMemory->bufferPos)[i]);
        sleep(1);
        sem_wait(sharedMemory->Mutex);
        /* Get the authority to write&read, then write&read data. */
        // First judge whether the buffer is empty now.
        if (Read_Number_FromBuff(sharedMemory->bufferPos, BUFFER_NUM_MAX - 1, &WritePos) > 0)
        {
            /* Read the corresponding data from the buff. */
            Read_Number_FromBuff(sharedMemory->bufferPos, WritePos - 1, &ReadData);
            Write_Number_IntoBuff(sharedMemory->bufferPos, BUFFER_NUM_MAX - 1, WritePos - 1);

            /* Judge whether the end comes. */
            if (ReadData == WRITE_NUM_MAX)
                sharedMemory->GetMaxNumber = true;

            /* Return the authority to read. */
            sem_post(sharedMemory->Mutex);

            /* Print the relative info. */
            printf("%6d\t\t%4d\t%d\t%d\n", getpid(), ReadData, WritePos, sharedMemory->GetMaxNumber);
        }
        else
        {
            /* Write the pos back. */
            Write_Number_IntoBuff(sharedMemory->bufferPos, BUFFER_NUM_MAX - 1, WritePos);
            /* Return the authority to read. */
            sem_post(sharedMemory->Mutex);
        }
    }
    /* Detach the shared memory. */
    if (shmdt(shm) == -1)
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    printf("\n\n\n\n Consumer PID: %d End! \n\n\n\n\n", getpid());
    return NULL;
}

int main(void)
{
    /* Create the buffer by creating a file. */
    FILE *filepointer = NULL; // The file pointer used for buffer.
    // Create the file.
    filepointer = fopen("buffer.txt", "rwx"); // The text file which could be written and read.
    if (!filepointer)
    {
        printf("filepointer error\n");
        exit(-1);
    }
    // Write numbers to the file.
    for (int i = 0; i < BUFFER_NUM_MAX; ++i)
        Write_Blank_IntoBuff(filepointer, i);
    Write_Number_IntoBuff(filepointer, BUFFER_NUM_MAX - 1, 0);
    // printf("Read Number: %d\n", Read_Number_FromBuff(filepointer, 4, NULL));
    fclose(filepointer);
#ifndef USING_FILE_OPERATION
    int *buffer = (int *)malloc(BUFFER_NUM_MAX * sizeof(int));
#endif

    /* Create the lock semaphore for the task. */
    sem_t *mutex, *full;
MUTEX_TRYAGAIN:
    mutex = (sem_t *)sem_open("mutex", O_CREAT, 0064, 1);
    if (!mutex)
    {
        printf("mutex sem error\n");
        sem_unlink("mutex");
        goto MUTEX_TRYAGAIN;
        exit(-1);
    }
FULL_TRYAGAIN:
    full = (sem_t *)sem_open("full", O_CREAT, 0064, 1);
    if (!full)
    {
        printf("full sem error\n");
        sem_unlink("full");
        goto FULL_TRYAGAIN;
        exit(-1);
    }

    /* Prepare for the shared data among threads. */
    /* Create shared memory. */
    int shmid = shmget((key_t)KEY_SHARED_MEMORY, sizeof(SharedData_t), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }
    void *shm = shmat(shmid, (void *)0, 0);
    if (shm == (void *)-1)
    {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    SharedData_t *sharedMemory = (SharedData_t *)shm;
    sharedMemory->Mutex = mutex;
    sharedMemory->full = full;
    sharedMemory->GetMaxNumber = false;
#ifndef USING_FILE_OPERATION
    sharedMemory->bufferPos = (void *)buffer;
#else
    sharedMemory.bufferPos = (void *)filepointer;
#endif

    /* Prepare the print message. */
    printf("PID\t\tData\n------\t\t----\n");

    /* Create five processes and record the PIDs. */
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork Failed");
        exit(-1);
    }
    else if (pid == 0)
    { // Child process.
        fork();
        fork();
        Consumer_Entry(); // Here comes four consumer processes.
        printf("1 End\n\n\n\n");
        while (wait(NULL) > 0)
            continue;
        exit(0);
    }
    else
    { // Father process.
        pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Fork Failed");
            exit(-1);
        }
        else if (pid)
            // Father process.
            Producer_Entry(sharedMemory); // Here comes the producer process.
        else if (!pid)
        {
            // Child process.
            Consumer_Entry(); // Here comes the fifth consumer process.
            printf("0 End\n\n\n\n");
            exit(0);
        }
    }

    /* Father process wait for all the child process to end. */
    while (wait(NULL) > 0)
        printf("waiting!\n");
    /* Detach the shared memory. */
    if (shmdt(shm) == -1)
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    /* Delete the semaphore. */
    sem_unlink("mutex");
    sem_unlink("full");

    return 0;
}
