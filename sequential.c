#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// #define STAMPA_MATRICE_A 2
// #define STAMPA_MATRICE_B 3

#define THRESHOLD 0.5


int main(int argc, char **argv) {

    if (argc != 2) 
    {
        printf("Error! Usage:  ./sequential N\n");
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]);
    int i, j, k, h;
    clock_t start, end;

    // Allocazione della matrice A NxN
    double** A = (double**) malloc(N * sizeof(double*));
    for(i = 0; i < N; i++) {
        A[i] = (double*) malloc (N * sizeof(double));
    }

    // Inizializzazione matrice A
    srand(time(0));
    for(i = 0; i < N; i++) {
        for(j = 0; j < N; j++) {
            A[i][j] = (double) rand() / RAND_MAX;
        }
    }

    // Stampa matrice A
    /* ==================================================== */
    #ifdef STAMPA_MATRICE_A
    printf("MATRICE A\n  ");
    for( i = 0; i < N; i++) {
        for(j = 0; j < N; j++) {
            printf("%lf  ", A[i][j]);             
        }
        printf("\n  ");
    }
    printf("\n\n");
    #endif
    /* ==================================================== */

    // Allocazione della matrice B -> di dimensione (N-2)x(N-2)
    int** B = (int**) malloc((N-2) * sizeof(int*));
    for(i = 0; i < N-2; i++) {
        B[i] = (int*) malloc ((N-2) * sizeof(int));
    }

    // Calcolo di B
    double somma;
    start = clock();
    for(i = 1; i < N-1; i++) {
        for(j = 1; j < N-1; j++) {

            somma = 0.0;

            for(k = -1; k <= 1; k++) {
                for(h = -1; h <= 1; h++) {
                    somma += A[i+k][j+h];
                }
            }

            if(somma / 9 < THRESHOLD)
                B[i-1][j-1] = 0;
            else
                B[i-1][j-1] = 1;
        }
    }
    end = clock();
  
    // Stampa matrice B
    /* ==================================================== */
    #ifdef STAMPA_MATRICE_B
    printf("\nMATRICE B\n  ");
    for( i = 0; i < N-2; i++) {
        for(j = 0; j < N-2; j++) {
            printf("%d  ", B[i][j]);             
        }
        printf("\n  ");
    }
    printf("\n\n");
    #endif
    /* ==================================================== */

    printf("Tempo di esecuzione = %lf s\n", ((double) (end - start)) / CLOCKS_PER_SEC);

    // Free
    for(i = 0; i < N; i++) {
        free(A[i]);
        if(i < N-2)
            free(B[i]);
    }
    free(A); 
    free(B);

    return 0;
}