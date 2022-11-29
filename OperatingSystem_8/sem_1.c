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

#define BUFFER_NUM_MAX 10
#define WRITE_NUM_MAX 9999

typedef struct
{
    bool GetMaxNumber; // The flag that represent the consumer has consume all the numbers.
    void *bufferPos;   // The buffer used among threads.
    sem_t *Mutex;      // The sem lock used for lock among threads.
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

void *Producer_Entry(void *param)
{
    SharedData_t *ShareData = (SharedData_t *)param;
    if (!ShareData)
        return NULL;
    int writePos = 0, WriteNumber = 0;
    while (WriteNumber <= WRITE_NUM_MAX)
    {
        /* Inquire the authority to write&read. */
        sem_wait(ShareData->Mutex);
        /* Get the authority to write&read, then write&read data. */
        // First judge whether the buffer is full now.
        if (Read_Number_FromBuff(ShareData->bufferPos, BUFFER_NUM_MAX - 1, &writePos) != BUFFER_NUM_MAX - 1)
        {
            // Write the corresponding number to the right position.
            Write_Number_IntoBuff(ShareData->bufferPos, writePos, WriteNumber);
            // Update the new valid position.
            Write_Number_IntoBuff(ShareData->bufferPos, BUFFER_NUM_MAX - 1, writePos + 1);
            // Increase the number to write.
            WriteNumber++;
        }
        else
        {
            // Now the buffer is full, thus nothing to operate but not write the number.
            Write_Number_IntoBuff(ShareData->bufferPos, BUFFER_NUM_MAX - 1, writePos);
        }
        /* Return the authority to write. */
        sem_post(ShareData->Mutex);
    }
    printf("\n\n\n\n Producer End! \n\n\n\n\n");
    return NULL;
}

void *Consumer_Entry(void *param)
{
    SharedData_t *ShareData = (SharedData_t *)param;
    if (!ShareData)
        return NULL;
    int WritePos = 0, ReadData = 0;
    while ((!ShareData->GetMaxNumber) || (WritePos > 0))
    {
        /* Inquire the authority to write&read. */
        sem_wait(ShareData->Mutex);
        /* Get the authority to write&read, then write&read data. */
        // First judge whether the buffer is empty now.
        if (Read_Number_FromBuff(ShareData->bufferPos, BUFFER_NUM_MAX - 1, &WritePos) > 0)
        {
            /* Read the corresponding data from the buff. */
            Read_Number_FromBuff(ShareData->bufferPos, WritePos - 1, &ReadData);
            Write_Number_IntoBuff(ShareData->bufferPos, BUFFER_NUM_MAX - 1, WritePos - 1);

            /* Judge whether the end comes. */
            if (ReadData == WRITE_NUM_MAX)
                ShareData->GetMaxNumber = true;

            /* Return the authority to read. */
            sem_post(ShareData->Mutex);

            /* Print the relative info. */
            printf("%6d\t\t%4d\n", getpid(), ReadData);
        }
        else
        {
            /* Write the pos back. */
            Write_Number_IntoBuff(ShareData->bufferPos, BUFFER_NUM_MAX - 1, WritePos);
            /* Return the authority to read. */
            sem_post(ShareData->Mutex);
        }
    }
    printf("\n\n\n\n Consumer End! \n\n\n\n\n");
    return NULL;
}

int main(void)
{
    /* Create the buffer by creating a file. */
    // Get the current location.
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
    // Create shared memeory as buffer.
    int segment_id = shmget(IPC_PRIVATE, BUFFER_NUM_MAX * sizeof(int), S_IRUSR | S_IWUSR);
    int *shared_memory = (int *)shmat(segment_id, NULL, 0);
#endif

    /* Create the lock semaphore for the task. */
    sem_t *mutex;
MUTEX_TRYAGAIN:
    mutex = (sem_t *)sem_open("mutex", O_CREAT, 0064, 1);
    if (!mutex)
    {
        printf("mutex sem error\n");
        sem_unlink("mutex");
        goto MUTEX_TRYAGAIN;
        exit(-1);
    }

    /* Prepare for the shared data among threads. */
    SharedData_t SharedData;
    SharedData.Mutex = mutex;
    SharedData.GetMaxNumber = false;
#ifndef USING_FILE_OPERATION
    SharedData.bufferPos = (void *)shared_memory;
#else
    SharedData.bufferPos = (void *)filepointer;
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
        Consumer_Entry(&SharedData); // Here comes four consumer processes.
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
            Producer_Entry(&SharedData); // Here comes the producer process.
        else if (!pid)
        {
            // Child process.
            Consumer_Entry(&SharedData); // Here comes the fifth consumer process.
            exit(0);
        }
    }

    /* Father process wait for all the child process to end. */
    while (wait(NULL) > 0)
        sleep(1);
    /* Delete the semaphore. */
    sem_unlink("mutex");
#ifndef USING_FILE_OPERATION
    shmdt(shared_memory);
    shmctl(segment_id, IPC_RMID, NULL);
#endif

    return 0;
}
