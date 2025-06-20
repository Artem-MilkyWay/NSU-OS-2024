#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_STEPS 200000000

typedef struct {
    int start;
    int end;
    double* partial_sum;  
} thread_args;

void* compute_partial_pi(void* arg) {
    thread_args* args = (thread_args*)arg;
    *(args->partial_sum) = 0.0;  
    
    for (int i = args->start; i < args->end; i++) {
        *(args->partial_sum) += 1.0 / (i * 4.0 + 1.0);
        *(args->partial_sum) -= 1.0 / (i * 4.0 + 3.0);
    }
    
    pthread_exit(args->partial_sum);  
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "use: %s <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "number of threads must be positive\n");
        return EXIT_FAILURE;
    }

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    thread_args* args = malloc(num_threads * sizeof(thread_args));
    double* partial_sums = malloc(num_threads * sizeof(double));  
    
    if (!threads || !args || !partial_sums) {
        perror("memory allocation failed");
        free(threads);
        free(args);
        free(partial_sums);
        return EXIT_FAILURE;
    }

    int base_steps = NUM_STEPS / num_threads;
    int extra_steps = NUM_STEPS % num_threads;
    int current_pos = 0;

    for (int i = 0; i < num_threads; i++) {
        args[i].start = current_pos;
        args[i].end = current_pos + base_steps + (i < extra_steps ? 1 : 0);
        args[i].partial_sum = &partial_sums[i];  
        current_pos = args[i].end;

        if (pthread_create(&threads[i], NULL, compute_partial_pi, &args[i])) {
            perror("failed to create thread");
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(args);
            free(partial_sums);
            return EXIT_FAILURE;
        }
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        void* ret;
        if (pthread_join(threads[i], &ret)) {
            perror("failed to join thread");
            free(threads);
            free(args);
            free(partial_sums);
            return EXIT_FAILURE;
        }
        pi += partial_sums[i]; 
    }

    pi *= 4.0;  

    printf("Pi = %.15f (using %d threads)\n", pi, num_threads);

    free(threads);
    free(args);
    free(partial_sums);
    return EXIT_SUCCESS;
}


