#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>

// #define DEBUG 1
// #define STAMPA_MATRICE_A 2
// #define STAMPA_MATRICE_B 3

#define THRESHOLD 0.5


int main(int argc, char **argv) {

    if (argc != 2) 
    {
        printf("Error! Usage:  ./versionMPI N\n");
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]);

    int my_rank, size;
    int i, j, k, h;
    double* rows_A;
    int* rows_B;
    double sum;
    int row_count;
    int* gather_rows;
    int* displacements;
    double start, end;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size > N-2) {
        printf("Errore: il numero di processi (=%d) deve essere minore di N-2 (=%d)\n", size, N-2);
        exit(EXIT_FAILURE);
    }

    if(my_rank == 0)
        printf("Inizio versioneMPI con %d processi su matrice %d x %d\n", size, N, N);

    int rows_per_thread = (N-2) / size;  
    int excess = (N-2) % size;


    /* ======================= Il master invia i chunk di A ai processi =======================*/


    if(my_rank == 0) {

        // Allocazione della matrice A -> dimensione NxN
        double* A = (double*) malloc(N * N * sizeof(double));

        // Inizializzazione matrice A
        srand(time(0));
        for( i = 0; i < N; i++) {
            for(j = 0; j < N; j++) {
               A[i*N + j] = (double) rand() / RAND_MAX;
            }
        }

        // Stampa matrice A
        /* ==================================================== */
        #ifdef STAMPA_MATRICE_A
        printf("MATRICE A\n  ");
        for( i = 0; i < N; i++) {
            for(j = 0; j < N; j++) {
                printf("%lf  ", A[i*N + j]);             
            }
            printf("\n  ");
        }
        printf("\n");
        #endif
        /* ==================================================== */

        // per ogni processo salvo il numero di elementi che deve restituire e il suo displacements
        gather_rows = (int*) malloc(size * sizeof(int));
        displacements = (int*) malloc(size * sizeof(int));

        row_count = 0;

        start = MPI_Wtime();
        for(int dest = 0; dest < size; dest++) {

            if(dest < excess) {
                // ai primi "excess" processi gli assegno rows_thread + 1 righe (+ 2 che corrispondono alla riga sopra e alla riga sotto)
                MPI_Send(A + (row_count * N), N * (rows_per_thread + 3), MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
                row_count += rows_per_thread + 1;
                gather_rows[dest] = (rows_per_thread + 1) * (N-2);
                if(dest == 0)
                    displacements[dest] = 0;
                else
                    displacements[dest] = displacements[dest - 1] + gather_rows[dest - 1];
            }
            else {
                // ai restanti solamente rows_thread righe
                MPI_Send(A + (row_count * N), N * (rows_per_thread + 2), MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
                row_count += rows_per_thread;
                gather_rows[dest] = rows_per_thread * (N-2);
                if(dest == 0)
                    displacements[dest] = 0;
                else
                    displacements[dest] = displacements[dest - 1] + gather_rows[dest - 1];
            }          
        }

        free(A);
    }


    /* ======================= I processi ricevono i chunk di A =======================*/
   
    if(my_rank < excess) {
        // Il processo riceve rows_thread + 3 righe
        rows_per_thread++; 
    }
    
    rows_A = (double*) malloc(N * (rows_per_thread + 2) * sizeof(double));
    rows_B = (int*) malloc((N-2) * rows_per_thread * sizeof(int));
    MPI_Recv(rows_A, N * (rows_per_thread + 2), MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    #ifdef DEBUG
    sleep(my_rank);
    printf("\n[Processo %d], ricevute %d righe della matrice A:\n", my_rank, rows_per_thread + 2);
    int r_count = 0;
    for(i = 0; i < N * (rows_per_thread + 2); i++) {
        printf("%lf  ", rows_A[i]);
        r_count++;
        if(r_count == N) {
            r_count = 0;
            printf("\n");
        }
    } 
    printf("[Processo %d] inizio calcolo\n", my_rank);
    #endif


    /* ========================== I processi eseguono il calcolo di B ===============================*/

    
    for(i = 1; i < rows_per_thread + 1; i++) {
        for(j = 1; j < N-1; j++) {
            // Calcolo di A[N + N*i + j], ovvero A[i][j]
            sum = 0;
            for(k = -1; k <= 1; k++) {
                for(h = -1; h <= 1; h++) {
                    sum += rows_A[(i+k)*N + j+h];
                }
            }

            if(sum / 9 < THRESHOLD)
                rows_B[((i-1) * (N-2)) + (j-1)] = 0;
            else
                rows_B[((i-1) * (N-2)) + (j-1)] = 1;               
        }
    }


    #ifdef DEBUG
    printf("[Processo %d] calcolata matrice B:\n", my_rank);
    for(i = 0; i < rows_per_thread; i++) {
        for(j = 0; j < N-2; j++)
                printf("%d ", rows_B[i * (N-2) + j]);              
        printf("\n");
    }
    #endif


    /* ===================== Ogni processo invia la parte di matrice B calcolata ======================= */

    if(my_rank == 0) {

        // Allocazione della matrice B -> di dimensione (N-2)x(N-2)
        int* B = (int*) malloc((N-2) * (N-2) * sizeof(int));

        #ifdef DEBUG
        sleep(size + 1);
        printf("[Processo %d] ricezione di \n   [ ", my_rank);
        for(int w = 0; w < size; w++) 
            printf("%d  ", gather_rows[w]);
        printf(" ]\n");
        printf("Con displacements \n   [ ");
        for(int w = 0; w < size; w++) 
            printf("%d  ", displacements[w]);
        printf(" ]\n");
        #endif


        MPI_Gatherv(rows_B, (N-2) * (rows_per_thread), MPI_INT, B, gather_rows, displacements, MPI_INT, 0, MPI_COMM_WORLD);
        end = MPI_Wtime();

        // Stampa matrice B
        // ============================================
        #ifdef STAMPA_MATRICE_B
        printf("\n[Processo %d] MATRICE B:\n", my_rank);
        for(i = 0; i < N-2; i++) {
            for(j = 0; j < N-2; j++) {
                printf("%d ", B[i*(N-2) + j]);
            }
            printf("\n");
        }
        #endif
        // ============================================ 

        printf("Tempo di esecuzione = %lf s\n\n", end - start);

        free(B);
    }
    else {
        MPI_Gatherv(rows_B, (N-2) * (rows_per_thread), MPI_INT, NULL, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
    }

    free(rows_A);
    free(rows_B);

    MPI_Finalize();
    return 0;

}