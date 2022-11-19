#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stropts.h>
#include <time.h>
#include <strings.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

// Specify that the communication between two processes uses the key value 23
#define MSGKEY 23

typedef enum
{
    From_P1_To_P2 = 1,
    From_P2_To_P1 = 2,
} Msg_Type_Enum; // Specify the message type number.

struct msg
{
    long msg_types;
    char msg_buf[511];
};

int main(void)
{
    /* Init the startup of the Experiment. */
    int qid, pid, len, retvalue;
    struct msg pmsg;
    if ((qid = msgget(MSGKEY, IPC_CREAT | 0666)) < 0) // Create the message object.
    {
        perror("msgget");
        exit(1);
    }

    /* Start to fork the child process. */
    pid = fork(); // Create the first child process.
    if (pid < 0)
    { // Fork failure.
        perror("fork");
        exit(1);
    }
    else if (pid != 0)
    { // Father process.
        pid = fork(); // Create the second child process.
        if(pid < 0)
        { // Fork precess.
            perror("fork");
            exit(1);
        }
        else if(pid != 0)
        { // Wait for all the child processes to end and be ready to return.
            wait(NULL);
            goto EXIT;
        }
        else if(pid == 0) // The second child process.
            goto P2;
    }
    else if(pid == 0)
        goto P1; // The first child process.

    /* Start of the work of each child process. */
P1: // The work of the process 1.
    // Write the message into the message.
    sprintf(pmsg.msg_buf, "hello! this is :%d\n\0", getpid());
    pmsg.msg_types = (int)From_P1_To_P2; // Set the message type.
    retvalue = 0;
    retvalue = msgsnd(qid, &pmsg, sizeof(pmsg.msg_buf), IPC_NOWAIT);
    if (retvalue < 0)
    {
        perror("msgsnd");
        exit(1);
    }
    printf("successfully send a message to the p2, destination qid:%d\n", qid);
    // Wait for the p2 to respond.
    retvalue = 0;
    while(!retvalue)
    {
        pmsg.msg_types = (int)From_P2_To_P1; // Set the message type!
        retvalue = msgrcv(qid, &pmsg, sizeof(pmsg.msg_buf), pmsg.msg_types, MSG_NOERROR);
    }
    // Print the data received.
    printf("p1 receiving message\n");
    printf("receive from p2: %s\n", pmsg.msg_buf);
    return 0;
P2: // The work of the process 2.
    // Wait for the p1 to start the communication.
    retvalue = 0;
    while(!retvalue)
    {
        pmsg.msg_types = (int)From_P1_To_P2; // Set the message type!
        retvalue = msgrcv(qid, &pmsg, sizeof(pmsg.msg_buf), pmsg.msg_types, MSG_NOERROR);
    }
    // Print the data received.
    printf("receive from p1: %s\n\n", pmsg.msg_buf);
    // Send data back to p1.
    // Write the message into the message.
    sprintf(pmsg.msg_buf, "hello! this is :%d\n\0", getpid());
    pmsg.msg_types = (int)From_P2_To_P1; // Set the message type.
    retvalue = 0;
    retvalue = msgsnd(qid, &pmsg, sizeof(pmsg.msg_buf), IPC_NOWAIT);
    if (retvalue < 0)
    {
        perror("msgsnd");
        exit(1);
    }
    printf("successfully send a message to the p1, destination qid:%d\n", qid);
    return 0;
    
    /* Delete the message created before. */
EXIT:
    msgctl(qid, IPC_RMID, NULL);
    return 0;
}
