#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    void *plib;
    typedef void (*FUN)(void);
    FUN fun = NULL;

    // Now could load the Dynamic Libaray, so should wait for the User to comfirm.
    printf("Before load dynamic libaray\nPlease enter to continue:");
    getchar();

    // Calculate and get the real library position.
    char *fileRelativePos = "/libtest.so";
    char *fileAbsolutePos = (char *)malloc(strlen(fileRelativePos) +
                                           strlen(getcwd(NULL, 0)));
    strcpy(fileAbsolutePos, getcwd(NULL, 0));
    strcat(fileAbsolutePos, fileRelativePos);
    // printf("The library direction: %s\n", fileAbsolutePos);
    // Open the Dynamic Libaray expected.
    plib = dlopen(fileAbsolutePos, RTLD_NOW | RTLD_GLOBAL);
    if (plib == NULL)
    {
        printf("error\n");
        exit(-1);
    }

    // Now load the Dynamic Libaray successfully, so should wait for the User to comfirm.
    fun = dlsym(plib, "printHello");
    if (fun)
        fun();

    // Now already release the Dynamic Libaray, so should wait for the User to comfirm.
    while (dlclose(plib))
        continue;
    fun = NULL;
    printf("After release dynamic libaray\nPlease enter to continue:");
    getchar();

    return 0;
}
