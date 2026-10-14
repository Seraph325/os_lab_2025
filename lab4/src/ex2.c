#include <stdio.h>
#include <unistd.h>

void main() {
    __pid_t child_pid = fork();

    int i = 0;

    if (child_pid >= 0) {
        if (child_pid != 0) {
            while(1) {
                printf("%d\n", ++i);
                sleep(1);
            }
        } else {
            i -= 1;
        }
    }

    printf("%d\n", i);
}