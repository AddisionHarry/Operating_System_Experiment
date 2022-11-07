#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    int i;
    for (i = 0; i < 2; i++) {
        fork();
        // printf("-\n");
        printf("-");
        fflush(stdout);
    }
    wait(NULL);
    wait(NULL);
    // printf("\n");
    return 0;
}
