#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mut1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut2 = PTHREAD_MUTEX_INITIALIZER;

void *thread_func1() {
    pthread_mutex_lock(&mut1);
    printf("THREAD1 LOCK MUT1\n");

    //for (int i = 0; i < 10000000; i++) { }

    pthread_mutex_lock(&mut2);
    printf("THREAD1 LOCK MUT2\n");

    pthread_mutex_unlock(&mut2);

    pthread_mutex_unlock(&mut1);

}

void *thread_func2() {
    pthread_mutex_lock(&mut2);
    printf("THREAD2 LOCK MUT2\n");

    //for (int i = 0; i < 10000000; i++) { }

    pthread_mutex_lock(&mut1);
    printf("THREAD2 LOCK MUT1\n");
    
    pthread_mutex_unlock(&mut2);
    
    pthread_mutex_unlock(&mut1);
}

int main() {
    pthread_t t1;
    pthread_t t2;

    pthread_create(&t1, NULL, thread_func1, NULL);
    pthread_create(&t2, NULL, thread_func2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}