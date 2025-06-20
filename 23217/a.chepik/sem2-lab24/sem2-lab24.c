#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define SEMAPHORE_NUM 4

sem_t semaphore[SEMAPHORE_NUM];

void destroy_semaphores(int end_indx) {
    for (int i = 0; i < end_indx; i++) {
        sem_destroy(&semaphore[i]);
    }
}

void init_semaphores() {
    int sc;

    for (int i = 0; i < SEMAPHORE_NUM; i++) {
        sc = sem_init(&semaphore[i], 0, 0);

        if (sc == -1) {
            printf("sem_init(&semaphore[%d], 0, 0) returned -1.\n", i);
            destroy_semaphores(i);

            exit(-1);
        }
    }
}

void pthread_create_handler(int code) {
    if (code != 0) {
        printf("pthread_create() returned a non-zero value.\n");
        destroy_semaphores(SEMAPHORE_NUM);

        exit(-1);
    }
}

void* make_A(void* arg) {
    sleep(1);
    printf("Detail A created\n");
    sem_post(&semaphore[0]);
    return NULL;
}

void* make_B(void* arg) {
    sleep(2);
    printf("Detail B created\n");
    sem_post(&semaphore[1]);
    return NULL;
}

void* make_C(void* arg) {
    sem_wait(&semaphore[2]);
    sleep(3);
    printf("Detail C created\n");
    sem_post(&semaphore[3]);
    return NULL;
}

void* make_module(void* arg) {
    sem_wait(&semaphore[0]);
    sem_wait(&semaphore[1]);
    printf("Module created\n");
    sem_post(&semaphore[2]);
    return NULL;
}

void* make_widget(void* arg) {
    sem_wait(&semaphore[3]);
    printf("Widget created\n");
    return NULL;
}

int main() {
    pthread_t a, b, c, module, widget;

    init_semaphores();

    pthread_create_handler(pthread_create(&a, NULL, make_A, NULL));
    pthread_create_handler(pthread_create(&b, NULL, make_B, NULL));
    pthread_create_handler(pthread_create(&c, NULL, make_C, NULL));
    pthread_create_handler(pthread_create(&module, NULL, make_module, NULL));
    pthread_create_handler(pthread_create(&widget, NULL, make_widget, NULL));

    pthread_join(widget, NULL);

    destroy_semaphores(SEMAPHORE_NUM);

    return 0;
}
