#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

//获取线程的调度算法
static int get_thread_policy(pthread_attr_t attr)
{
    int policy;
    pthread_attr_getschedpolicy(&attr, &policy);
    switch (policy)
    {
    //实时调度策略 先到先服务
    case SCHED_FIFO:
        printf("policy = SCHED_FIFO\n");
        break;
    //时间片轮转调度算法
    case SCHED_RR:
        printf("policy = SCHED_RR\n");
        break;
    //分时调度算法
    case SCHED_OTHER:
        printf("policy = SCHED_OTHER\n");
        break;
    default:
        printf("policy = UNKOWN\n");
        break;
    }
    return policy;
}

//获取指定调度算法可以设置的最大/最小优先级
static void show_thread_priority(pthread_attr_t attr, int policy)
{
    int priority = sched_get_priority_max(policy);
    printf("max_priority = %d\n", priority);
    priority = sched_get_priority_min(policy);
    printf("min_priority = %d\n", priority);
}
//获取给定线程的优先级
static int get_thread_priority(pthread_attr_t attr)
{
    struct sched_param param;
    pthread_attr_getschedparam(&attr, &param);
    printf("priority = %d\n", param.sched_priority);
    return param.sched_priority;
}
//设置线程的调度算法
static void set_thread_policy(pthread_attr_t attr, int policy)
{
    pthread_attr_setschedpolicy(&attr, policy);
    get_thread_policy(attr);
}

void *thread_entry(void *param)
{
    pthread_attr_t attr;
    /* get the default attributes*/
    pthread_attr_init(&attr);

    /* Get the current schedule policy. */
    int policy = get_thread_policy(attr);
    /* Get the max and min priority in this policy. */
    printf("Show current configuration of priority.\n");
    show_thread_priority(attr, policy);
    /* Show the other two policy's configuration of priority */
    if (policy != SCHED_FIFO)
    {
        printf("Show SCHED_FIFO of priority.\n");
        show_thread_priority(attr, SCHED_FIFO);
    }
    if (policy != SCHED_RR)
    {
        printf("Show SCHED_RR of priority.\n");
        show_thread_priority(attr, SCHED_RR);
    }
    if (policy != SCHED_OTHER)
    {
        printf("Show SCHED_OTHER of priority.\n");
        show_thread_priority(attr, SCHED_OTHER);
    }
    /* Get the priority of the current thread */
    printf("Show priority of current thread.\n");
    get_thread_priority(attr);
    /* Set the policy of the current thread*/
    if (policy != SCHED_FIFO)
    {
        set_thread_policy(attr, SCHED_FIFO);
        printf("Set THRRAD SCHED_FIFO\n");
    }
    if (policy != SCHED_RR)
    {
        set_thread_policy(attr, SCHED_RR);
        printf("Set THRRAD SCHED_RR\n");
    }
    if (policy != SCHED_OTHER)
    {
        set_thread_policy(attr, SCHED_OTHER);
        printf("Set THRRAD SCHED_OTHER\n");
    }
    /* Restore the policy of the current thread. */
    printf("Restore current policy.\n");
    set_thread_policy(attr, policy);
}

int main(void)
{
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr); // get default setting
    pthread_create(&tid, &attr, thread_entry, NULL);

    pthread_exit(0);
}
