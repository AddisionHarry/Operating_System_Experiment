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

#define MSGKEY 123

struct msg
{
    long msg_types;
    char msg_buf[511];
};

int main(int argc, char **argv)
{
    struct msg pmsg;
    int msqid, pid, retvalue;
    msqid = msgget(MSGKEY, IPC_CREAT | IPC_EXCL);
    if (msqid < 0)
    {
        perror("msgget");
        exit(-1);
    }
    pid = fork();
    if (pid > 0)
    {
        // Father process
        int s;
    A:
        printf("Input msg type:\n");
        scanf("%d", &s);
        if (!s)
        {
            printf("Valid type! Please try again!\n");
            goto A;
        }
        pmsg.msg_types = s;
        sprintf(pmsg.msg_buf, "hello! this is :%d\n\0", getpid());
        retvalue = msgsnd(msqid, &pmsg, sizeof(pmsg.msg_buf), IPC_NOWAIT);
        if (retvalue < 0)
        {
            perror("magsnd");
            exit(-1);
        }
    }
    else if (pid == 0)
    {
        // Child process
        pmsg.msg_types = 1;
        retvalue = msgrcv(msqid, &pmsg, sizeof(pmsg.msg_buf), pmsg.msg_types, MSG_NOERROR);
        if (retvalue > 0)
            printf("read msg:%s\n", pmsg.msg_buf);
        exit(0);
    }
    wait(NULL);
    msgctl(msqid, IPC_RMID, NULL);
    return 0;
}
