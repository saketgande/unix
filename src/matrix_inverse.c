#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

#define MAX_SIZE 4096

typedef double matrix[MAX_SIZE][MAX_SIZE];

int N; // Matrix size
int maxnum; // Maximum element value
char* Init; // Initialization type
int PRINT; // Print flag
matrix A, I; // Matrices A and I (inverse of A)

pthread_barrier_t barrier;

// Structure for thread arguments
typedef struct {
    int start_row;
    int end_row;
} ThreadArg;

// Function prototypes
void* thread_function(void* arg);
void find_inverse();
void Init_Matrix();
void Print_Matrix(matrix M, char* name);
void Init_Default();
int Read_Options(int argc, char** argv);

int main(int argc, char** argv) {
    Init_Default(); // Initialize default values
    Read_Options(argc, argv); // Read command-line options
    Init_Matrix(); // Initialize matrix A and I

    find_inverse(); // Perform matrix inversion

    if (PRINT == 1) {
        Print_Matrix(I, "Inversed Matrix");
    }

    return 0;
}

void* thread_function(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;

    for (int p = 0; p < N; ++p) {
        double pivalue = A[p][p];
        for (int col = 0; col < N; ++col) {
            if (p >= thread_arg->start_row && p < thread_arg->end_row) {
                A[p][col] /= pivalue;
                I[p][col] /= pivalue;
            }
        }

        pthread_barrier_wait(&barrier);

        for (int row = thread_arg->start_row; row < thread_arg->end_row; ++row) {
            if (row != p) {
                double multiplier = A[row][p];
                for (int col = 0; col < N; ++col) {
                    A[row][col] -= A[p][col] * multiplier;
                    I[row][col] -= I[p][col] * multiplier;
                }
            }
        }

        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

void find_inverse() {
    int P = 4; // Number of threads
    pthread_t threads[P];
    ThreadArg thread_args[P];

    pthread_barrier_init(&barrier, NULL, P);

    for (int i = 0; i < P; ++i) {
        thread_args[i].start_row = i * N / P;
        thread_args[i].end_row = (i + 1) * N / P;
        pthread_create(&threads[i], NULL, thread_function, &thread_args[i]);
    }

    for (int i = 0; i < P; ++i) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
}

void Init_Matrix() {
    printf("\nInitializing matrix...\n");
    printf("Matrix size = %dx%d\n", N, N);
    printf("Max element value = %d\n", maxnum);
    printf("Initialization type = %s\n", Init);

    // Initialize matrix A and I
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            if (strcmp(Init, "rand") == 0) {
                A[row][col] = (row == col) ? (double)(rand() % maxnum) + 5.0 : (double)(rand() % maxnum) + 1.0;
            } else { // "fast"
                A[row][col] = (row == col) ? 5.0 : 2.0;
            }
            I[row][col] = (row == col) ? 1.0 : 0.0;
        }
    }
}

void Print_Matrix(matrix M, char* name) {
    printf("%s:\n", name);
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            printf(" %5.2f", M[row][col]);
        }
        printf("\n");
    }
    printf("\n");
}

void Init_Default() {
    N = 5; // Default matrix size
    Init = "fast"; // Default initialization type
    maxnum = 15; // Default maximum number
    PRINT = 1; // Default print flag
}

int Read_Options(int argc, char** argv) {
    char* prog = argv[0];
    while (++argv, --argc > 0) {
        if (**argv == '-') {
            switch (*++*argv) {
                case 'n': N = atoi(*++argv); --argc; break;
                case 'I': Init = *++argv; --argc; break;
                case 'm': maxnum = atoi(*++argv); --argc; break;
                case 'P': PRINT = atoi(*++argv); --argc; break;
                case 'h':
                case 'u':
                    printf("Usage: %s [-n size] [-I init_type] [-m maxnum] [-P print_flag]\n", prog);
                    exit(0);
                    break;
            }
        }
    }
    return 0;
}