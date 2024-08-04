#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// #define DEBUG 1
// #define STAMPA_MATRICE_A 2
// #define STAMPA_MATRICE_B 3

#define THRESHOLD 0.5

int main(int argc, char **argv) {

    if (argc != 3) 
    {
        printf("Error! Usage:  ./versionOpenMP N p\n");
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]);
    int p = atoi(argv[2]);

    int i, j, k, h;
    double start, end;

    // Allocazione della matrice A -> dimensione NxN
    float* A = (float*) malloc(N * N * sizeof(float));

    // Allocazione della matrice B -> di dimensione (N-2)x(N-2)
    int* B = (int*) malloc((N-2) * (N-2) * sizeof(int));

    // Inizializzazione matrice A
    srand(time(0));
    for(i = 0; i < N; i++) {
        for(j = 0; j < N; j++) {
            A[i*N + j] = (float) rand() / (float) RAND_MAX;
        }
    }

    // Stampa matrice A
    /* ==================================================== */
    #ifdef STAMPA_MATRICE_A
    printf("MATRICE A\n  ");
    for( i = 0; i < N; i++) {
        for(j = 0; j < N; j++) {
            printf("%f  ", A[i*N + j]);             
        }
        printf("\n  ");
    }
    printf("\n\n");
    #endif
    /* ==================================================== */

    // Calcolo di B
    float somma;
    printf("Inizio calcolo su matrice A = %dx%d, con %d thread\n", N, N, p);
    start = omp_get_wtime(); 


    #pragma omp parallel for num_threads(p) schedule(static, 1) private(somma)  
        for(int i = 1; i < N-1; i++) {
            for(int j = 1; j < N-1; j++) {

                somma = 0.0F;

                for(int k = -1; k <= 1; k++) {
                    for(int h = -1; h <= 1; h++) {
                        somma += A[(i+k)*N + j+h];
                    }
                }   
               
                if(somma / 9 < 0.5)
                    B[(i-1)*(N-2) + (j-1)] = 0;
                else
                     B[(i-1)*(N-2) + (j-1)] = 1;
            }
        }
    
    end = omp_get_wtime(); 

    // Stampa matrice B
    /* ==================================================== */
    #ifdef STAMPA_MATRICE_B
    printf("\nMATRICE B\n  ");
    for( i = 0; i < N-2; i++) {
        for(j = 0; j < N-2; j++) {
            printf("%d  ", B[i*(N-2) + j]);             
        }
        printf("\n  ");
    }
    printf("\n\n");
    #endif
    /* ==================================================== */

    printf("Tempo di esecuzione = %lf s\n", end - start);

    free(A); 
    free(B);

    return 0;
}