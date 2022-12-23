#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef enum
{
    FileLock = 0,  // The semaphore is used for file lock,
    Producing_End, // The semaphore that indicate all the number has been produced.
    WritingPos,    // The semaphore that indicate on which pos is empty.

    Sem_All // The number of the semaphore used.
} Sem_Purpose_Enum;

#define USE_SEMAPHORE 1               // Whether to use the semaphore
#define BUFFER_FILE_NAME "buffer.txt" // The name of the buffer file.
#define BUFFER_MAX_NUMBERS 10         // The most numbers buffer could hold.
#define BUFFER_MAX_NUMBER 99999       // The max number buffer could hold.
#define FTOK_PROJ_ID 16               // The proj_id used in ftok().
#define SEM_SET_NUMBER (int)Sem_All   // The number of semaphores in the sem set.

/**
 * @brief Get the file-lock Sem ID
 * @author fwlh
 * @param  argv             The file path.
 * @return int              The sem id get.
 */
int Get_SemID(char *argv)
{
#if USE_SEMAPHORE
    // Get the semaphore key.
    int key = ftok(argv, FTOK_PROJ_ID);
    if (key == -1)
    {
        fprintf(stderr, "Error in ftok:%s!\n", strerror(errno));
        exit(1);
    }
    // Create semaphore.
    int semid = semget(key, SEM_SET_NUMBER, IPC_CREAT | 0666);
    if (semid == -1)
    {
        fprintf(stderr, "Error in semget:%s\n", strerror(errno));
        exit(1);
    }
    return semid;
#else
    return 0;
#endif
}

/**
 * @brief Set the value of the single semaphore.
 * @author fwlh
 * @param  semid            The id of the semaphore set.
 * @param  index            The index of the semaphore to set value.
 * @param  value            The value to set.
 */
void Set_Single_Semaphore_Value(int semid, int index, int value)
{
#if USE_SEMAPHORE
    // Invalid input index.
    if ((index < 0) || (index >= SEM_SET_NUMBER))
        return;
    if (semctl(semid, index, SETVAL, &value) == -1)
        fprintf(stderr, "Error in semget:%s\n", strerror(errno));
#endif
}

/**
 * @brief Set the value of all the semaphore.
 * @author fwlh
 * @param  semid            The id of the semaphore set.
 * @param  value            The value to set, the param should be an array
 *                              if SEM_SET_NUMBER does not equal 1
 *                              and the array size should be SEM_SET_NUMBER
 */
void Set_All_Semaphore_Value(int semid, unsigned short *value)
{
#if USE_SEMAPHORE
    if (!value)
        printf("Set a null pointer!\n");
    union semun
    {
        int val;
        struct semid_ds *buf;
        unsigned short *arry;
        struct seminfo *__buf;
    } arg;
    if (SEM_SET_NUMBER == 1)
    {
        arg.val = (int)*value;
        if (semctl(semid, 0, SETVAL, arg) == -1)
            fprintf(stderr, "Error in semget:%s\n", strerror(errno));
    }
    else
    {
        arg.arry = value;
        if (semctl(semid, 0, SETALL, arg) == -1)
            fprintf(stderr, "Error in semget:%s\n", strerror(errno));
    }
#endif
}

/**
 * @brief Release the specific semaphore in the sem set.
 * @author fwlh
 * @param  semid            The sem set id
 * @param  index            The index of the specific sem.
 * @return int              Whether the operation is performed successfully.
 */
int Semaphore_Release(int semid, int index)
{
#if USE_SEMAPHORE
    struct sembuf sops[SEM_SET_NUMBER];
    sops[index].sem_num = index;
    sops[index].sem_op = +1;
    sops[index].sem_flg = SEM_UNDO;
    return semop(semid, sops, SEM_SET_NUMBER);
#else
    return 0;
#endif
}

/**
 * @brief Take the specific semaphore in the sem set.
 * @author fwlh
 * @param  semid            The sem set id
 * @param  index            The index of the specific sem.
 * @return int              Whether the operation is performed successfully.
 */
int Semaphore_Take(int semid, int index)
{
#if USE_SEMAPHORE
    struct sembuf sops[SEM_SET_NUMBER];
    sops[index].sem_num = index;
    sops[index].sem_op = -1;
    sops[index].sem_flg = SEM_UNDO;
    return semop(semid, sops, SEM_SET_NUMBER);
#else
    return 0;
#endif
}

/**
 * @brief Convert a string into a int number.
 * @author fwlh
 * @param  string           The string to convert.
 * @param  len              The length of the string
 * @return int              The final integer number.
 */
static int String2IntNum_CVT(char *string, int len)
{
    int Number = 0;
    for (int i = 0; i < len; ++i)
    {
        if (string[i] == ' ')
            break;
        else if ((string[i] <= '9') && (string[i] >= '0'))
        {
            Number *= 10;
            Number += string[i] - '0';
        }
    }
    return Number;
}
/**
 * @brief Read a number from the buffer.
 * @author fwlh
 * @param  index            The real pos need to read.
 * @return int              The number read from the buffer.
 */
int Read_Number_From_Buffer(int index)
{
    // The index is not valid.
    if ((index < 0) || (index >= BUFFER_MAX_NUMBERS))
        return -1;
    // Read the number.
    int fd = open(BUFFER_FILE_NAME, O_RDWR);
    const char blank[6] = "     ";
    char buf[6];
    lseek(fd, 6 * index, SEEK_SET);
    if (read(fd, buf, 5) < 0)
        fprintf(stderr, "Error in semget:%s\n", strerror(errno));
    buf[5] = 0;
    lseek(fd, 6 * index, SEEK_SET);
    write(fd, blank, 5);
    close(fd);
    // Convert the string into number.
    return String2IntNum_CVT(buf, 5);
}

/**
 * @brief Write a number to the specific position of the buffer.
 * @author fwlh
 * @param  Number           The number to be written, the number should be in
 *                             range [0, BUFFER_MAX_NUMBER]
 * @param  index            The pos to write into, could set range
 *                             [0, BUFFER_MAX_NUMBERS - 1]
 */
void Write_Number_Into_Buffer(int Number, int index)
{
    // The index is not valid.
    if ((index < 0) || (index >= BUFFER_MAX_NUMBERS))
        return;
    // The number is too large.
    else if ((Number < 0) || (Number > BUFFER_MAX_NUMBER))
        return;
    int fd = open(BUFFER_FILE_NAME, O_RDWR);
    char buf[6];
    char const blank[6] = "     ";
    sprintf(buf, "%5d", Number);
    lseek(fd, 6 * index, SEEK_SET);
    write(fd, buf, 5);
    close(fd);
}

typedef enum
{
    Read_From_Buffer = 0,
    Write_Into_Buffer
} Buffer_Operation_Enum;

/**
 * @brief  Get the read/write pointer of the buffer.
 * @author fwlh
 * @param  semid            The necessary semaphore set id.
 * @param  operation        The operation to inquire.
 * @return int              The read/write pointer.
 */
static int Get_ReadWrite_Pos(int semid, Buffer_Operation_Enum operation)
{
    switch (operation)
    {
    case Read_From_Buffer:
        return (9 - semctl(semid, (int)WritingPos, GETVAL));
    case Write_Into_Buffer:
        return (10 - semctl(semid, (int)WritingPos, GETVAL));
    default:
        return 0;
    }
}

/**
 * @brief Operate the read/write pointer.
 * @author fwlh
 * @param  semid            The necessary semaphore set id.
 * @param  operation        The operation just done.
 */
static void Sem_Operate_Buffer(int semid, Buffer_Operation_Enum operation)
{
    switch (operation)
    {
    case Read_From_Buffer:
        Semaphore_Release(semid, (int)WritingPos);
        break;
    case Write_Into_Buffer:
        Semaphore_Take(semid, (int)WritingPos);
        break;
    default:
        return;
    }
}

/**
 * @brief The entry of the consumer process.
 * @author fwlh
 */
void Consumer_Entry(char *path)
{
    // Get the semaphore.
    int semid = Get_SemID(path);
    int Read_Pos = 0; // The pos to be read.

    while (1)
    {
        // Check whether the buffer is empty.
        if (Get_ReadWrite_Pos(semid, Read_From_Buffer) == -1)
        {
            // If the producer is not producing now, then could end itself.
            if (semctl(semid, (int)Producing_End, GETVAL))
                return;
            continue;
        }
        // Try to get the file locker.
        Semaphore_Take(semid, (int)FileLock);
        // Check again whether the buffer could be read.
        if ((Read_Pos = Get_ReadWrite_Pos(semid, Read_From_Buffer)) != -1)
        {
            // Read a number.
            Sem_Operate_Buffer(semid, Read_From_Buffer);
            Read_Number_From_Buffer(Read_Pos);
        }
        // Release the file locker.
        Semaphore_Release(semid, (int)FileLock);
    }
}

/**
 * @brief The entry of the producer process.
 * @author fwlh
 */
void Producer_Entry(char *path, int maxProduce)
{
    // Get the semaphore.
    int semid = Get_SemID(path);
    int produce_cnt = 0; // The number produced now.
    int write_pos = 0;   // The position in the buffer to write.

    while (1)
    {
        // Judge whether the produce target is over.
        if (produce_cnt == maxProduce)
        {
            Semaphore_Release(semid, (int)Producing_End);
            break;
        }
        // Judge whether the buffer could hold to produce.
        if (Get_ReadWrite_Pos(semid, Write_Into_Buffer) == BUFFER_MAX_NUMBERS)
            continue;
        // Try to get the file locker.
        Semaphore_Take(semid, (int)FileLock);
        // Recheck now could write to the buffer.
        if ((write_pos = Get_ReadWrite_Pos(semid, Write_Into_Buffer)) !=
            BUFFER_MAX_NUMBERS)
        {
            // Write the number.
            Sem_Operate_Buffer(semid, Write_Into_Buffer);
            // Write_Number_Into_Buffer(produce_cnt, write_pos);
            ++produce_cnt;
        }
        // Release the file locker.
        Semaphore_Release(semid, (int)FileLock);
    }
}

/**
 * @brief Fork 6 processes.
 * @author fwlh
 * @return int            return 1 if the father process, 2-7 the child processes,
 *                              -1 if error.
 */
static int fork_6(void)
{
    for (int ret_number_child = 2; ret_number_child <= 7; ++ret_number_child)
    {
        int pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Fork Failed");
            return -1;
        }
        else if (pid == 0)
            // The first child process.
            return ret_number_child;
    }
    return 1;
}

/**
 * @brief Create the buffer.txt and write the ',' into the file to
 * partition the numbers.
 * @author fwlh
 */
void Write_File_Start(void)
{
    int fd = creat(BUFFER_FILE_NAME, O_CREAT | 0666);
    ftruncate(fd, 0); // Clear the hole file.
    // Open the file, but only need to write.
    fd = open(BUFFER_FILE_NAME, O_WRONLY);
    char blank[7] = "     ,";
    for (int i = 0; i < BUFFER_MAX_NUMBERS; ++i)
    {
        // Write the file.
        lseek(fd, 6 * i, SEEK_SET);
        if (i != BUFFER_MAX_NUMBERS - 1)
            write(fd, blank, strlen(blank));
        else
            write(fd, blank, strlen(blank) - 1);
    }
    // Don't forget to close.
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Please give the produce number!\n");
        exit(-1);
    }
    int Produce_Number = String2IntNum_CVT(argv[1], strlen(argv[1]));

    /* The first step: Init the necessary kernel object. */
    // Create the buffer file and write the necessary content.
    Write_File_Start();
    // Create the semaphore.
    int semid = Get_SemID(argv[0]);
    // Set the initial value.
    unsigned short value[SEM_SET_NUMBER] =
        {[FileLock] = 1,
         [Producing_End] = 0,
         [WritingPos] = 10};
    Set_All_Semaphore_Value(semid, value);

    /* The second step: Create six processes. */
    int ret = fork_6();
    // printf("ret = %d\n", ret);
    if (ret < 0)
        exit(-1);
    else if (ret == 1)
    {
        // Wait for all the child processes to end.
        for (int i = 0; i < 6; ++i)
            wait(NULL);
        goto EXIT;
    }
    // Call the producer and consumer processes respectively.
    else if (ret == 2)
    {
        Producer_Entry(argv[0], Produce_Number);
        exit(0);
    }
    else
    {
        // Consumer_Entry(argv[0]);
        exit(0);
    }

EXIT:
    // Ready to delete the semaphore.
    semid = Get_SemID(argv[0]);
    semctl(semid, 0, IPC_RMID);

    return 0;
}
