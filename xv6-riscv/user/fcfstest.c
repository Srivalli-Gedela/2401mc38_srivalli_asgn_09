#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
    int i;
    int j;
    int pid;

    for(i = 0; i < 3; i++){
        pid = fork();

        if(pid == 0){
            for(j = 0; j < 50000000; j++)
                ;

            printf("PID %d completed\n", getpid());
            exit(0);
        }

        for(j = 0; j < 1000000; j++)
            ;
    }

    for(i = 0; i < 3; i++)
        wait(0);

    exit(0);
}
