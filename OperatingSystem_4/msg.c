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

struct msg
{
    long msg_types;
    char msg_buf[511];
};

int main(void)
{
    int qid, pid, len;
    struct msg pmsg;
    sprintf(pmsg.msg_buf, "hello! this is :%d\n\0", getpid());
    if ((qid = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) < 0)
    {
        perror("msgget");
        exit(1);
    }
    pid = fork(); // Create a new process.
    if (pid < 0)
    { // Fork failure.
        perror("fork");
    }
    else if (pid != 0)
    { // Father process.
        //     pmsg.msg_types = 1;
        //     if ((msgsnd(qid, &pmsg, len, 0)) < 0)
        //     {
        //         perror("msgsnd");
        //         exit(1);
        //     }
        //     printf("successfully send a message to the queue: %d\n", qid);
        //     wait(NULL);
        //     printf("\n\nFather process:pid = %d\n", getpid());
        //     len = msgrcv(qid+1, &pmsg, 512, 0, MSG_NOERROR);
        //     if(len == -1)
        //     {
        //         perror("msgrcv");
        //         exit(1);
        //     }
        //     printf("receive from msg: msg.types = %d, content:\t", pmsg.msg_types);
        //     printf(pmsg.msg_buf);
        //     // Delete msg.
        //     msgctl(qid, IPC_RMID, NULL);
        //     msgctl(qid+1, IPC_RMID, NULL);
        // }
        // else if(pid == 0)
        // { // Child process.
        //     printf("\n\nChild process:pid = %d\n", getpid());
        //     len = msgrcv(qid, &pmsg, 512, 0, MSG_NOERROR);
        //     if(len == -1)
        //     {
        //         perror("msgrcv");
        //         exit(1);
        //     }
        //     printf("receive from msg: msg.types = %d, content:\t", pmsg.msg_types);
        //     printf(pmsg.msg_buf);
        //     // Send msg.
        //     if ((qid = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) < 0 )
        //     {
        //         perror("msgget");
        //         exit (1);
        //     }
        //     sprintf(pmsg.msg_buf, "hello! this is :%d\n\0", getpid());
        //     pmsg.msg_types = 1;
        //     if ((msgsnd(qid, &pmsg, len, 0)) < 0)
        //     {
        //         perror("msgsnd");
        //         exit(1);
        //     }
        //     printf("successfully send a message to the queue: %d\n", qid);
        // }
        return 0;
    }
